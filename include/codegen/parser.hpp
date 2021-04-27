#ifndef _PAREAS_CODEGEN_PARSER_HPP
#define _PAREAS_CODEGEN_PARSER_HPP

#include "codegen/token.hpp"
#include "codegen/datatype.hpp"

#include <vector>

class Lexer;
class ASTNode;
class SymbolTable;

class Parser {
    private:
        Lexer& lexer;
        SymbolTable& symtab;

        void expect(TokenType);

        ASTNode* parseAssign();
        ASTNode* parseCompare();
        ASTNode* parseAdd();
        ASTNode* parseMul();
        ASTNode* parseCast();
        ASTNode* parseAtom();
        ASTNode* parseExpression();
        ASTNode* parseExpressionStatement();
        ASTNode* parseIfElseStatement();
        ASTNode* parseWhileStatement();
        ASTNode* parseStatement();
        ASTNode* parseStatementList();
        ASTNode* parseArgumentList(std::vector<DataType>&);
        ASTNode* parseFunction();
        ASTNode* parseFunctionList();
    public:
        Parser(Lexer&, SymbolTable&);

        ASTNode* parse();
};

#endif
