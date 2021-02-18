#ifndef _PAREAS_CODEGEN_DEPTHTREE_HPP
#define _PAREAS_CODEGEN_DEPTHTREE_HPP

#include <cstdint>
#include <cstddef>
#include <iosfwd>
#include <unordered_map>

class ASTNode;

class DepthTree {
    private:
        uint8_t* node_types;
        uint8_t* resulting_types;
        uint32_t* parents;
        uint32_t* depth;
        uint32_t* child_idx;
        int64_t* instr_offsets;
        size_t max_nodes;
        size_t filled_nodes;
        size_t max_depth;

        void construct(ASTNode*);
        void setElement(size_t, ASTNode*, size_t, size_t, size_t);
        void markOffset(ASTNode*, const std::unordered_map<ASTNode*, size_t>&, size_t&);
    public:
        DepthTree(size_t elems, ASTNode*);
        ~DepthTree();

        void print(std::ostream&) const;

        inline const uint8_t* getNodeTypes() const {
            return this->node_types;
        }
        inline const uint8_t* getResultingTypes() const {
            return this->resulting_types;
        }
        inline const uint32_t* getParents() const {
            return this->parents;
        }
        inline const uint32_t* getDepth() const {
            return this->depth;
        }
        inline const uint32_t* getChildren() const {
            return this->child_idx;
        }
        inline const int64_t* getInstrOffsets() const {
            return this->instr_offsets;
        }
        inline size_t maxNodes() const {
            return this->max_nodes;
        }
        inline size_t maxDepth() const {
            return this->max_depth;
        }
};

std::ostream& operator<<(std::ostream&, const DepthTree&);

#endif
