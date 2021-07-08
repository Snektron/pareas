#ifndef _PAREAS_TEST_ASTNODE_HPP_
#define _PAREAS_TEST_ASTNODE_HPP_

#include <vector>
#include <cstdint>

#include "test/nodetype.hpp"
#include "test/datatype.hpp"

struct ASTNode {
    NodeType node_type;
    DataType data_type;
    std::vector<ASTNode*> children;
    uint32_t node_data;

    inline ~ASTNode() {
        for(ASTNode* n : this->children) {
            delete n;
        }
    }
};

#endif