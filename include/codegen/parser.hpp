#ifndef _PAREAS_CODEGEN_PARSER_HPP
#define _PAREAS_CODEGEN_PARSER_HPP

#include "codegen/token.hpp"

class Lexer;
class ASTNode;
class SymbolTable;

class Parser {
    private:
        Lexer& lexer;
        SymbolTable& symtab;

        void expect(TokenType);

        ASTNode* parseAdd();
        ASTNode* parseMul();
        ASTNode* parseCast();
        ASTNode* parseAtom();
        ASTNode* parseExpression();
        ASTNode* parseExpressionStatement();
        ASTNode* parseStatementList();
    public:
        Parser(Lexer&, SymbolTable&);

        ASTNode* parse();
};

#endif
