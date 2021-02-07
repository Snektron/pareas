#include "codegen/parser.hpp"
#include "codegen/lexer.hpp"
#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"

#include <memory>
#include <vector>

Parser::Parser(Lexer& lexer) : lexer(lexer) {}

void Parser::expect(TokenType token_type) {
    Token token = this->lexer.lex();

    if(token.type != token_type) {
        throw ParseException("Parsing failed, expecting ", token_type, " got ", token);
    }
}

ASTNode* Parser::parseExpression() {
    this->lexer.lex();
    return new ASTNode(NodeType::INVALID);
}

ASTNode* Parser::parseExpressionStatement() {
    std::unique_ptr<ASTNode> res(this->parseExpression());
    this->expect(TokenType::SEMICOLON);
    return res.release();
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