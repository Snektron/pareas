#ifndef _PAREAS_TEST_DEFAULTTREEPRINTER_HPP_
#define _PAREAS_TEST_DEFAULTTREEPRINTER_HPP_

#include <iosfwd>

#include "test/treeprinter.hpp"

class DefaultTreePrinter : public TreePrinter {
    private:
        std::ostream& os;
        size_t indent = 0;

        void printIndent();
    public:
        DefaultTreePrinter(std::ostream&);

        void print(ASTNode*);
};

#endif