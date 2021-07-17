#ifndef _PAREAS_TREEPROPERTIES_HPP_
#define _PAREAS_TREEPROPERTIES_HPP_

#include <cstddef>

class ASTNode;

class TreeProperties {
    private:
        ASTNode* root;
    public:
        TreeProperties(ASTNode*);

        size_t getNodeCount() const;
        size_t getDepth() const;
        size_t getWidth() const;
        size_t getFunctions() const;
        size_t getMaxFuncLen() const;
};

#endif