#include "codegen/depthtree.hpp"
#include "codegen/astnode.hpp"

#include <queue>
#include <limits>
#include <tuple>
#include <cstring>
#include <iostream>

DepthTree::DepthTree(size_t max_nodes, ASTNode* node) : max_nodes(max_nodes) {
    this->node_types = new uint8_t[max_nodes];
    this->resulting_types = new uint8_t[max_nodes];
    this->parents = new uint32_t[max_nodes];
    this->depth = new uint32_t[max_nodes];

    std::memset(this->node_types, 0, sizeof(uint8_t) * max_nodes);
    std::memset(this->resulting_types, 0, sizeof(uint8_t) * max_nodes);
    std::memset(this->parents, -1, sizeof(uint32_t) * max_nodes);
    std::memset(this->depth, -1, sizeof(uint32_t) * max_nodes);

    this->construct(node);
}

DepthTree::~DepthTree() {
    delete[] this->node_types;
    delete[] this->resulting_types;
    delete[] this->parents;
    delete[] this->depth;
}

void DepthTree::setElement(size_t idx, ASTNode* node, size_t parent, size_t depth) {
    this->node_types[idx] = static_cast<uint8_t>(node->getType());
    this->resulting_types[idx] = static_cast<uint8_t>(node->getResultingType());
    this->parents[idx] = parent;
    this->depth[idx] = depth;
}

void DepthTree::construct(ASTNode* node) {
    std::queue<std::tuple<ASTNode*, size_t, size_t>> search_queue;
    search_queue.push(std::tuple<ASTNode*, size_t, size_t>(node, std::numeric_limits<size_t>::max(), 0));

    size_t i = 0;
    while(!search_queue.empty()) {
        auto [n, parent, depth] = search_queue.front();
        search_queue.pop();

        this->setElement(i, n, parent, depth);

        for(ASTNode* c : n->getChildren()) {
            search_queue.push(std::tuple<ASTNode*, size_t, size_t>(c, i, depth + 1));
        }

        ++i;
    }

    this->filled_nodes = i;
}

void DepthTree::print(std::ostream& os) const {
    for(size_t i = 0; i < this->filled_nodes; ++i) {
        os << "Node " << i << ", node type = " << static_cast<unsigned>(this->node_types[i])
            << ", data type = " << static_cast<unsigned>(this->resulting_types[i])
            << ", parent = " << this->parents[i] << ", depth = " << this->depth[i] << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const DepthTree& tree) {
    tree.print(os);
    return os;
}