#ifndef _PAREAS_TEST_SOURCEFILETRINTER_HPP_
#define _PAREAS_TEST_SOURCEFILEPRINTER_HPP_

#include <iosfwd>

#include "test/treeprinter.hpp"
#include "test/datatype.hpp"

class SourceFilePrinter : public TreePrinter {
    private:
        std::ostream& os;
        size_t indent = 0;

        void printIndent();
        void printFuncDecl(ASTNode*);
        void printFuncArgList(ASTNode*);
        void printStatementList(ASTNode*);
        void printStatement(ASTNode*);
        void printReturnStatement(ASTNode*);
        void printConditional(ASTNode*);
        void printBinaryExpression(ASTNode*);
        void printUnaryExpression(ASTNode*);
        void printAssignExpression(ASTNode*);
        void printCast(ASTNode*);
        void printLiteral(ASTNode*);
        void printDeclaration(ASTNode*);
        void printCall(ASTNode*);
        void printId(ASTNode*);
        void printDataType(DataType);
    public:
        SourceFilePrinter(std::ostream&);

        void print(ASTNode*);
};

#endif