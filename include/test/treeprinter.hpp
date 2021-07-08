#ifndef _PAREAS_TEST_TREEPRINTER_HPP_
#define _PAREAS_TEST_TREEPRINTER_HPP_

class ASTNode;

class TreePrinter {
    public:
        virtual ~TreePrinter() = default;
        virtual void print(ASTNode*) = 0;
};

#endif