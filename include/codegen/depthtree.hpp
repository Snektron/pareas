#ifndef _PAREAS_CODEGEN_DEPTHTREE_HPP
#define _PAREAS_CODEGEN_DEPTHTREE_HPP

#include <cstdint>
#include <cstddef>
#include <iosfwd>

class ASTNode;

class DepthTree {
    private:
        uint8_t* node_types;
        uint8_t* resulting_types;
        uint32_t* parents;
        uint32_t* depth;
        size_t max_nodes;
        size_t filled_nodes;

        void construct(ASTNode*);
        void setElement(size_t, ASTNode*, size_t, size_t);
    public:
        DepthTree(size_t elems, ASTNode*);
        ~DepthTree();

        void print(std::ostream&) const;
};

std::ostream& operator<<(std::ostream&, const DepthTree&);

#endif
