#include "codegen/parser.hpp"
#include "codegen/lexer.hpp"
#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"

#include <memory>
#include <vector>

NodeType bin_node_type_for(TokenType type) {
    switch(type) {
        case TokenType::PLUS:
            return NodeType::ADD_EXPR;
        case TokenType::MIN:
            return NodeType::SUB_EXPR;
        default:
            return NodeType::INVALID;
    }
}

Parser::Parser(Lexer& lexer) : lexer(lexer) {}

void Parser::expect(TokenType token_type) {
    Token token = this->lexer.lex();

    if(token.type != token_type) {
        throw ParseException("Parsing failed, expecting ", token_type, " got ", token);
    }
}

ASTNode* Parser::parseAdd() {
    std::unique_ptr<ASTNode> lop(this->parseCast());

    Token lookahead = this->lexer.lookahead();
    while(lookahead.type == TokenType::PLUS || lookahead.type == TokenType::MIN) {
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
        case TokenType::INTEGER:
            return new ASTNode(NodeType::LIT_EXPR, DataType::INT, lookahead.integer);
        default:
            throw ParseException("Parsing failed, unexpected token ", lookahead, ", expecting atom");
    }
}

ASTNode* Parser::parseExpression() {
    return this->parseAdd();
}

ASTNode* Parser::parseExpressionStatement() {
    std::unique_ptr<ASTNode> res(this->parseExpression());
    this->expect(TokenType::SEMICOLON);
    return new ASTNode(NodeType::EXPR_STAT, {res.release()});
}

ASTNode* Parser::parseStatementList() {
    std::vector<std::unique_ptr<ASTNode>> nodes;

    bool active = true;
    while(active) {
        Token lookahead = this->lexer.lookahead();

        switch(lookahead.type) {
            case TokenType::SEMICOLON:
                this->expect(TokenType::SEMICOLON);
                nodes.emplace_back(new ASTNode(NodeType::EMPTY_STAT));
                break;
            case TokenType::PLUS:
            case TokenType::MIN:
            case TokenType::OPEN_PAR:
            case TokenType::INT:
            case TokenType::FLOAT:
            case TokenType::INTEGER:
            case TokenType::ID:
                nodes.emplace_back(this->parseExpressionStatement());
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

ASTNode* Parser::parse() {
    return this->parseStatementList();
}