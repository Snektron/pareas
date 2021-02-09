#include "codegen/depthtree.hpp"

DepthTree::DepthTree(size_t max_nodes) : max_nodes(max_nodes) {
    this->node_types = new uint8_t[max_nodes];
    this->resulting_types = new uint8_t[max_nodes];
    this->parents = new uint32_t[max_nodes];
    this->depth = new uint32_t[max_nodes];
}

DepthTree::~DepthTree() {
    delete[] this->node_types;
    delete[] this->resulting_types;
    delete[] this->parents;
    delete[] this->depth;
}