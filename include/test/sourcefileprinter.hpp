#ifndef _PAREAS_TEST_SOURCEFILETRINTER_HPP_
#define _PAREAS_TEST_SOURCEFILEPRINTER_HPP_

#include <iosfwd>

#include "test/treeprinter.hpp"

class SourceFilePrinter : public TreePrinter {
    private:
        std::ostream& os;
        size_t indent = 0;

        void printIndent();
        void printStatementList(ASTNode*);
        void printStatement(ASTNode*);
        void printBinaryExpression(ASTNode*);
        void printUnaryExpression(ASTNode*);
        void printLiteral(ASTNode*);
    public:
        SourceFilePrinter(std::ostream&);

        void print(ASTNode*);
};

#endif