#include "codegen/parser.hpp"
#include "codegen/lexer.hpp"
#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"
#include "codegen/symtab.hpp"

#include <memory>
#include <vector>

NodeType bin_node_type_for(TokenType type) {
    switch(type) {
        case TokenType::PLUS:
            return NodeType::ADD_EXPR;
        case TokenType::MIN:
            return NodeType::SUB_EXPR;
        case TokenType::MUL:
            return NodeType::MUL_EXPR;
        case TokenType::DIV:
            return NodeType::DIV_EXPR;
        case TokenType::MOD:
            return NodeType::MOD_EXPR;
        case TokenType::EQ:
            return NodeType::EQ_EXPR;
        case TokenType::NEQ:
            return NodeType::NEQ_EXPR;
        case TokenType::LESS:
            return NodeType::LESS_EXPR;
        case TokenType::GREATER:
            return NodeType::GREAT_EXPR;
        case TokenType::LESSEQ:
            return NodeType::LESSEQ_EXPR;
        case TokenType::GREATEQ:
            return NodeType::GREATEQ_EXPR;
        default:
            return NodeType::INVALID;
    }
}

Parser::Parser(Lexer& lexer, SymbolTable& symtab) : lexer(lexer), symtab(symtab) {}

void Parser::expect(TokenType token_type) {
    Token token = this->lexer.lex();

    if(token.type != token_type) {
        throw ParseException("Parsing failed, expecting ", token_type, " got ", token);
    }
}

ASTNode* Parser::parseAssign() {
    std::unique_ptr<ASTNode> lop(this->parseCompare());

    Token lookahead = this->lexer.lookahead();
    if(lookahead.type == TokenType::ASSIGN) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseAssign());
        return new ASTNode(NodeType::ASSIGN_EXPR, {lop.release(), rop.release()});
    }
    return lop.release();
}

ASTNode* Parser::parseCompare() {
    std::unique_ptr<ASTNode> lop(this->parseAdd());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::EQ || lookahead.type == TokenType::NEQ ||
            lookahead.type == TokenType::GREATER || lookahead.type == TokenType::LESS ||
            lookahead.type == TokenType::GREATEQ || lookahead.type == TokenType::LESSEQ) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseAdd());
        lop.reset(new ASTNode(bin_node_type_for(lookahead.type), {lop.release(), rop.release()}));

        lookahead = this->lexer.lookahead();
    }

    return lop.release();
}

ASTNode* Parser::parseAdd() {
    std::unique_ptr<ASTNode> lop(this->parseMul());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::PLUS || lookahead.type == TokenType::MIN) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseMul());
        lop.reset(new ASTNode(bin_node_type_for(lookahead.type), {lop.release(), rop.release()}));

        lookahead = this->lexer.lookahead();
    }

    return lop.release();
}

ASTNode* Parser::parseMul() {
    std::unique_ptr<ASTNode> lop(this->parseCast());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::MUL || lookahead.type == TokenType::DIV || lookahead.type == TokenType::MOD) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseCast());
        lop.reset(new ASTNode(bin_node_type_for(lookahead.type), {lop.release(), rop.release()}));

        lookahead = this->lexer.lookahead();
    }

    return lop.release();
}

ASTNode* Parser::parseCast() {
    std::unique_ptr<ASTNode> lop(this->parseAtom());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::CAST) {
        this->lexer.lex();

        Token token = this->lexer.lex();
        DataType d;
        switch(token.type) {
            case TokenType::INT:
                d = DataType::INT;
                break;
            case TokenType::FLOAT:
                d = DataType::FLOAT;
                break;
            default:
                throw ParseException("Parsing failed, unexpected token ", token, ", expecting typename");
        }

        lop.reset(new ASTNode(NodeType::CAST_EXPR, d, {lop.release()}));

        lookahead = this->lexer.lookahead();
    }

    return lop.release();
}

ASTNode* Parser::parseAtom() {
    Token lookahead = this->lexer.lex();
    switch(lookahead.type) {
        case TokenType::OPEN_PAR: {
            std::unique_ptr<ASTNode> expr(this->parseExpression());
            this->expect(TokenType::CLOSE_PAR);
            return expr.release();
        }
        case TokenType::ID: {
            std::string id = lookahead.str;
            lookahead = this->lexer.lookahead();
            if(lookahead.type == TokenType::DECL) {
                this->lexer.lex();
                lookahead = this->lexer.lex();
                DataType symbol_type;
                switch(lookahead.type) {
                    case TokenType::INT:
                        symbol_type = DataType::INT_REF;
                        break;
                    case TokenType::FLOAT:
                        symbol_type = DataType::FLOAT_REF;
                        break;
                    default:
                        throw ParseException("Parsing failed, unexpected token ", lookahead, ", expecting typename");
                }
                uint32_t symbol_id = this->symtab.declareSymbol(id, symbol_type);
                return new ASTNode(NodeType::DECL_EXPR, symbol_type, symbol_id);
            }
            else if(lookahead.type == TokenType::OPEN_PAR) {
                this->lexer.lex();
                lookahead = this->lexer.lookahead();

                std::vector<std::unique_ptr<ASTNode>> params;
                
                if(lookahead.type != TokenType::CLOSE_PAR) {
                    params.emplace_back(this->parseExpression());
                    lookahead = this->lexer.lookahead();
                    while(lookahead.type == TokenType::COMMA) {
                        this->lexer.lex();
                        params.emplace_back(this->parseExpression());
                        lookahead = this->lexer.lookahead();
                    }
                }

                this->expect(TokenType::CLOSE_PAR);
                //TODO: return the call
            }
            else {
                Symbol symbol = this->symtab.resolveSymbol(id);
                return new ASTNode(NodeType::ID_EXPR, symbol.type, symbol.id);
            }
        }
        case TokenType::INTEGER:
            return new ASTNode(NodeType::LIT_EXPR, DataType::INT, lookahead.integer);
        case TokenType::FLOAT:
            return new ASTNode(NodeType::LIT_EXPR, DataType::FLOAT, lookahead.integer);
        default:
            throw ParseException("Parsing failed, unexpected token ", lookahead, ", expecting atom");
    }
}

ASTNode* Parser::parseExpression() {
    return this->parseAssign();
}

ASTNode* Parser::parseExpressionStatement() {
    std::unique_ptr<ASTNode> res(this->parseExpression());
    this->expect(TokenType::SEMICOLON);
    return new ASTNode(NodeType::EXPR_STAT, {res.release()});
}

ASTNode* Parser::parseIfElseStatement() {
    this->expect(TokenType::IF);
    std::unique_ptr<ASTNode> cond(this->parseExpression());
    std::unique_ptr<ASTNode> stat(this->parseStatement());

    Token lookahead = this->lexer.lookahead();
    if(lookahead.type == TokenType::ELSE) {
        this->expect(TokenType::ELSE);
        std::unique_ptr<ASTNode> else_stat(this->parseStatement());
        return new ASTNode(NodeType::IF_ELSE_STAT, {cond.release(), stat.release(), else_stat.release()});
    }
    else {
        return new ASTNode(NodeType::IF_STAT, {cond.release(), stat.release()});
    }
}

ASTNode* Parser::parseWhileStatement() {
    this->expect(TokenType::WHILE);
    std::unique_ptr<ASTNode> cond(this->parseExpression());
    std::unique_ptr<ASTNode> stat(this->parseStatement());
    return new ASTNode(NodeType::WHILE_STAT, {new ASTNode(NodeType::WHILE_DUMMY), cond.release(), stat.release()});
}

ASTNode* Parser::parseStatement() {
    Token lookahead = this->lexer.lookahead();

    switch(lookahead.type) {
        case TokenType::SEMICOLON:
            this->expect(TokenType::SEMICOLON);
            return new ASTNode(NodeType::EMPTY_STAT);
        case TokenType::IF:
            return this->parseIfElseStatement();
        case TokenType::PLUS:
        case TokenType::MIN:
        case TokenType::OPEN_PAR:
        case TokenType::INTEGER:
        case TokenType::FLOAT:
        case TokenType::ID:
            return this->parseExpressionStatement();
        case TokenType::OPEN_CB: {
            std::unique_ptr<ASTNode> list(this->parseStatementList());
            this->expect(TokenType::CLOSE_CB);
            return list.release();
        }
        case TokenType::WHILE:
            return this->parseWhileStatement();
        default:
            throw ParseException("Parsing failed, unexpected token ", lookahead, ", expecting start of statement");
    }
}

ASTNode* Parser::parseStatementList() {
    std::vector<std::unique_ptr<ASTNode>> nodes;

    bool active = true;
    while(active) {
        Token lookahead = this->lexer.lookahead();

        switch(lookahead.type) {
            case TokenType::SEMICOLON:
            case TokenType::IF:
            case TokenType::PLUS:
            case TokenType::MIN:
            case TokenType::OPEN_PAR:
            case TokenType::INTEGER:
            case TokenType::FLOAT:
            case TokenType::ID:
            case TokenType::OPEN_CB:
            case TokenType::WHILE:
                nodes.emplace_back(this->parseStatement());
                break;
            default:
                active = false;
                break;
        }
    }

    std::vector<ASTNode*> node_list;
    for(size_t i = 0; i < nodes.size(); ++i) {
        node_list.push_back(nodes[i].release());
    }

    return new ASTNode(NodeType::STATEMENT_LIST, node_list);
}

ASTNode* Parser::parseFunction() {
    this->expect(TokenType::FUNCTION);

    Token id = this->lexer.lex();
    if(id.type != TokenType::ID)
        throw ParseException("Parsing failed, unexpected token ", id, ", expecting identifier");

    std::vector<DataType> arg_types;
    this->expect(TokenType::OPEN_PAR);
    this->expect(TokenType::CLOSE_PAR);
    this->expect(TokenType::DECL);

    DataType return_type;
    Token type = this->lexer.lex();
    switch(type.type) {
        case TokenType::INT:
            return_type = DataType::INT;
            break;
        case TokenType::FLOAT:
            return_type = DataType::FLOAT;
            break;
        default:
            throw ParseException("Parsing failed, unexpected token ", id, ", expecting typename");
    }

    uint32_t symbol_id = this->symtab.declareFunction(id.str, return_type, arg_types);
    this->symtab.newFunction();

    this->expect(TokenType::OPEN_CB);
    std::unique_ptr<ASTNode> function_body(this->parseStatementList());
    this->expect(TokenType::CLOSE_CB);

    this->symtab.endFunction();

    return new ASTNode(NodeType::FUNC_DECL, return_type, symbol_id, {function_body.release()});
}

ASTNode* Parser::parseFunctionList() {
    Token id = this->lexer.lookahead();
    std::vector<std::unique_ptr<ASTNode>> nodes;
    while(id.type == TokenType::FUNCTION) {
        nodes.emplace_back(this->parseFunction());
        id = this->lexer.lookahead();
    }

    std::vector<ASTNode*> result;
    for(size_t i = 0; i < nodes.size(); ++i)
        result.push_back(nodes[i].release());

    return new ASTNode(NodeType::STATEMENT_LIST, result);
}

ASTNode* Parser::parse() {
    std::unique_ptr<ASTNode> result(this->parseFunctionList());

    this->expect(TokenType::END_OF_FILE);
    return result.release();
}