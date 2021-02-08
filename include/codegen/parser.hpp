#ifndef _PAREAS_CODEGEN_PARSER_HPP
#define _PAREAS_CODEGEN_PARSER_HPP

#include "codegen/token.hpp"

class Lexer;
class ASTNode;

class Parser {
    private:
        Lexer& lexer;

        void expect(TokenType);

        ASTNode* parseAdd();
        ASTNode* parseCast();
        ASTNode* parseAtom();
        ASTNode* parseExpression();
        ASTNode* parseExpressionStatement();
        ASTNode* parseStatementList();
    public:
        Parser(Lexer&);

        ASTNode* parse();
};

#endif
