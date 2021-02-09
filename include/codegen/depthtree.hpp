#ifndef _PAREAS_CODEGEN_DEPTHTREE_HPP
#define _PAREAS_CODEGEN_DEPTHTREE_HPP

#include "codegen/astnode.hpp"

class DepthTree {
    private:
        uint8_t* node_types;
        uint8_t* resulting_types;
        uint32_t* parents;
        uint32_t* depth;
        size_t max_nodes;
    public:
        DepthTree(size_t elems);
        ~DepthTree();
};

#endif
