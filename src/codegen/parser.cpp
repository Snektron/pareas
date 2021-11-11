#include "codegen/parser.hpp"
#include "codegen/lexer.hpp"
#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"
#include "codegen/symtab.hpp"

#include <memory>
#include <vector>
#include <iostream>

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
        case TokenType::BITAND:
            return NodeType::BITAND_EXPR;
        case TokenType::BITOR:
            return NodeType::BITOR_EXPR;
        case TokenType::BITXOR:
            return NodeType::BITXOR_EXPR;
        case TokenType::LAND:
            return NodeType::LAND_EXPR;
        case TokenType::LOR:
            return NodeType::LOR_EXPR;
        case TokenType::LSHIFT:
            return NodeType::LSHIFT_EXPR;
        case TokenType::RSHIFT:
            return NodeType::RSHIFT_EXPR;
        case TokenType::URSHIFT:
            return NodeType::URSHIFT_EXPR;
        default:
            return NodeType::INVALID;
    }
}

NodeType un_node_type_for(TokenType type) {
    switch(type) {
        case TokenType::MIN:
            return NodeType::NEG_EXPR;
        case TokenType::NOT:
            return NodeType::LNOT_EXPR;
        case TokenType::BITNOT:
            return NodeType::BITNOT_EXPR;
        default:
            return NodeType::INVALID;
    }
}

Parser::Parser(Lexer& lexer, SymbolTable& symtab) : lexer(lexer), symtab(symtab) {}

void Parser::expect(TokenType token_type) {
    Token token = this->lexer.lex();

    if(token.type != token_type) {
        throw ParseException("Parsing failed at line ", this->lexer.line(), ", expecting ", token_type, " got ", token);
    }
}

ASTNode* Parser::parseAssign() {
    std::unique_ptr<ASTNode> lop(this->parseLogical());

    Token lookahead = this->lexer.lookahead();
    if(lookahead.type == TokenType::ASSIGN) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseAssign());
        return new ASTNode(NodeType::ASSIGN_EXPR, {lop.release(), rop.release()});
    }
    return lop.release();
}

ASTNode* Parser::parseLogical() {
    std::unique_ptr<ASTNode> lop(this->parseBitwise());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::LAND || lookahead.type == TokenType::LOR) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseBitwise());
        lop.reset(new ASTNode(bin_node_type_for(lookahead.type), {lop.release(), rop.release()}));

        lookahead = this->lexer.lookahead();
    }

    return lop.release();
}

ASTNode* Parser::parseBitwise() {
    std::unique_ptr<ASTNode> lop(this->parseCompare());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::BITAND || lookahead.type == TokenType::BITOR || lookahead.type == TokenType::BITXOR) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseCompare());
        lop.reset(new ASTNode(bin_node_type_for(lookahead.type), {lop.release(), rop.release()}));

        lookahead = this->lexer.lookahead();
    }

    return lop.release();
}

ASTNode* Parser::parseCompare() {
    std::unique_ptr<ASTNode> lop(this->parseShift());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::EQ || lookahead.type == TokenType::NEQ ||
            lookahead.type == TokenType::GREATER || lookahead.type == TokenType::LESS ||
            lookahead.type == TokenType::GREATEQ || lookahead.type == TokenType::LESSEQ) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseShift());
        lop.reset(new ASTNode(bin_node_type_for(lookahead.type), {lop.release(), rop.release()}));

        lookahead = this->lexer.lookahead();
    }

    return lop.release();
}

ASTNode* Parser::parseShift() {
    std::unique_ptr<ASTNode> lop(this->parseAdd());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::LSHIFT || lookahead.type == TokenType::RSHIFT || lookahead.type == TokenType::URSHIFT) {
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
    std::unique_ptr<ASTNode> lop(this->parseUnary());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::MUL || lookahead.type == TokenType::DIV || lookahead.type == TokenType::MOD) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> rop(this->parseUnary());
        lop.reset(new ASTNode(bin_node_type_for(lookahead.type), {lop.release(), rop.release()}));

        lookahead = this->lexer.lookahead();
    }

    return lop.release();
}

ASTNode* Parser::parseUnary() {
    Token lookahead = this->lexer.lookahead();

    if(lookahead.type == TokenType::MIN || lookahead.type == TokenType::NOT || lookahead.type == TokenType::BITNOT) {
        this->lexer.lex();

        std::unique_ptr<ASTNode> op(this->parseUnary());
        std::unique_ptr<ASTNode> result(new ASTNode(un_node_type_for(lookahead.type), {op.release()}));

        return result.release();
    }
    return this->parseCast();
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
                throw ParseException("Parsing failed at line ", this->lexer.line(), ", unexpected token ", token, ", expecting typename");
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
                        throw ParseException("Parsing failed at line ", this->lexer.line(), ", unexpected token ", lookahead, ", expecting typename");
                }
                uint32_t symbol_id = this->symtab.declareSymbol(id, symbol_type);
                return new ASTNode(NodeType::DECL_EXPR, symbol_type, symbol_id);
            }
            else if(lookahead.type == TokenType::OPEN_PAR) {
                this->lexer.lex();
                lookahead = this->lexer.lookahead();

                std::vector<std::unique_ptr<ASTNode>> params;
                
                if(lookahead.type != TokenType::CLOSE_PAR) {
                    std::unique_ptr<ASTNode> expr1(this->parseExpression());
                    params.emplace_back(new ASTNode(NodeType::FUNC_CALL_ARG, {expr1.release()}));
                    lookahead = this->lexer.lookahead();
                    while(lookahead.type == TokenType::COMMA) {
                        this->lexer.lex();
                        std::unique_ptr<ASTNode> expr(this->parseExpression());
                        params.emplace_back(new ASTNode(NodeType::FUNC_CALL_ARG, {expr.release()}));
                        lookahead = this->lexer.lookahead();
                    }
                }

                this->expect(TokenType::CLOSE_PAR);

                std::vector<ASTNode*> arg_list;
                for(size_t i = 0; i < params.size(); ++i) {
                    arg_list.push_back(params[i].release());
                }

                std::unique_ptr<ASTNode> arg_list_node(new ASTNode(NodeType::FUNC_CALL_ARG_LIST, arg_list));

                uint32_t func_id = this->symtab.resolveFunction(id);
                DataType ret_type = this->symtab.getFunctionReturnType(func_id);
                return new ASTNode(NodeType::FUNC_CALL_EXPR, ret_type, func_id, {arg_list_node.release()});
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
            throw ParseException("Parsing failed at line ", this->lexer.line(), ", unexpected token ", lookahead, ", expecting atom");
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

ASTNode* Parser::parseReturnStatement() {
    this->expect(TokenType::RETURN);
    Token lookahead = this->lexer.lookahead();
    uint32_t func_id = this->symtab.getCurrentFunction();
    if(lookahead.type == TokenType::SEMICOLON) {
        this->expect(TokenType::SEMICOLON);
        return new ASTNode(NodeType::RETURN_STAT, DataType::VOID, func_id, {});
    }
    else {
        std::unique_ptr<ASTNode> expr(this->parseExpression());
        this->expect(TokenType::SEMICOLON);
        return new ASTNode(NodeType::RETURN_STAT, DataType::VOID, func_id, {expr.release()});
    }
}

ASTNode* Parser::parseStatement() {
    Token lookahead = this->lexer.lookahead();

    switch(lookahead.type) {
        case TokenType::SEMICOLON:
            this->expect(TokenType::SEMICOLON);
            return new ASTNode(NodeType::EMPTY_STAT);
        case TokenType::IF:
            return this->parseIfElseStatement();
        case TokenType::MIN:
        case TokenType::BITNOT:
        case TokenType::NOT:
        case TokenType::OPEN_PAR:
        case TokenType::INTEGER:
        case TokenType::FLOAT:
        case TokenType::ID:
            return this->parseExpressionStatement();
        case TokenType::OPEN_CB: {
            this->symtab.enterScope();
            this->expect(TokenType::OPEN_CB);
            std::unique_ptr<ASTNode> list(this->parseStatementList());
            this->expect(TokenType::CLOSE_CB);
            this->symtab.exitScope();
            return list.release();
        }
        case TokenType::WHILE:
            return this->parseWhileStatement();
        case TokenType::RETURN:
            return this->parseReturnStatement();
        default:
            throw ParseException("Parsing failed at line ", this->lexer.line(), ", unexpected token ", lookahead, ", expecting start of statement");
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
            case TokenType::MIN:
            case TokenType::BITNOT:
            case TokenType::NOT:
            case TokenType::OPEN_PAR:
            case TokenType::INTEGER:
            case TokenType::FLOAT:
            case TokenType::ID:
            case TokenType::OPEN_CB:
            case TokenType::WHILE:
            case TokenType::RETURN:
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

ASTNode* Parser::parseArgumentList(std::vector<DataType>& arg_types) {
    this->expect(TokenType::OPEN_PAR);

    std::vector<std::unique_ptr<ASTNode>> arguments;
    Token lookahead = this->lexer.lookahead();

    size_t num_int_args = 0;

    auto parse_decl = [&] () {
        Token current = this->lexer.lex();
        if(current.type != TokenType::ID)
            throw ParseException("Parsing failed at line ", this->lexer.line(), ", unexpected token ", current, ", expecting identifier");
        
        std::string var_name = current.str;

        this->expect(TokenType::DECL);

        current = this->lexer.lex();
        DataType res_type;
        switch(current.type) {
            case TokenType::INT:
                res_type = DataType::INT_REF;
                break;
            case TokenType::FLOAT:
                res_type = DataType::FLOAT_REF;
                break;
            default:
                throw ParseException("Parsing failed at line", this->lexer.line(), ", unexpected token ", current, ", expecting typename");
        }
        uint32_t symbol_id = this->symtab.declareSymbol(var_name, res_type);
        std::unique_ptr<ASTNode> decl_node(new ASTNode(NodeType::DECL_EXPR, res_type, symbol_id));
        size_t arg_idx = res_type == DataType::INT_REF ? num_int_args++ : arguments.size() - num_int_args;
        arguments.emplace_back(new ASTNode(NodeType::FUNC_ARG, DataType::INVALID, arg_idx, {decl_node.release()}));
    };

    if(lookahead.type != TokenType::CLOSE_PAR) {
        parse_decl();
        Token lookahead = this->lexer.lookahead();
        while(lookahead.type == TokenType::COMMA) {
            this->lexer.lex();
            parse_decl();
            lookahead = this->lexer.lookahead();
        }
    }

    this->expect(TokenType::CLOSE_PAR);

    std::vector<ASTNode*> node_list;
    for(size_t i = 0; i < arguments.size(); ++i) {
        node_list.push_back(arguments[i].release());
    }

    return new ASTNode(NodeType::FUNC_ARG_LIST, node_list);
}

ASTNode* Parser::parseFunction() {
    this->expect(TokenType::FUNCTION);

    Token id = this->lexer.lex();
    if(id.type != TokenType::ID)
        throw ParseException("Parsing failed at line ", this->lexer.line(), ", unexpected token ", id, ", expecting identifier");

    this->symtab.newFunction();
    this->symtab.enterScope();

    std::vector<DataType> arg_types;
    std::unique_ptr<ASTNode> argument_list(this->parseArgumentList(arg_types));
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
        case TokenType::VOID:
            return_type = DataType::VOID;
            break;
        default:
            throw ParseException("Parsing failed at line ", this->lexer.line(), ", unexpected token ", type, ", expecting typename");
    }

    uint32_t symbol_id = this->symtab.declareFunction(id.str, return_type, arg_types);

    this->expect(TokenType::OPEN_CB);
    std::unique_ptr<ASTNode> function_body(this->parseStatementList());
    this->expect(TokenType::CLOSE_CB);

    this->symtab.exitScope();
    this->symtab.endFunction();

    return new ASTNode(NodeType::FUNC_DECL, return_type, symbol_id, {new ASTNode(NodeType::FUNC_DECL_DUMMY), argument_list.release(), function_body.release()});
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