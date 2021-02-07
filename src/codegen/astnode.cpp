#include "codegen/astnode.hpp"

#include <iostream>

const char* NODE_NAMES[] = {
    "invalid",
    "statement list",
    "empty statement",
    "expression statement",
    "if statement",
    "if-else statement",
    "else auxiliary",
    "while statement",
    "function call expression",
    "function call argument",
    "add expression",
    "sub expression",
    "mul expression",
    "div expression",
    "mod expression",
    "bitwise and expression",
    "bitwise or expression",
    "bitwise xor expression",
    "lshift expression",
    "rshift expression",
    "urshift expression",
    "logical and expression",
    "logical or expression",
    "equals expression",
    "not equals expression",
    "less expression",
    "greater expression",
    "less equal expression",
    "greater equal expression",
    "bitwise not expression",
    "logical not expression",
    "negate expression",
    "literal expression",
    "cast expression",
    "assignment expression",
    "declaration expression",
    "identifier expression"
};

ASTNode::ASTNode(NodeType type) : type(type) {}
ASTNode::ASTNode(NodeType type, const std::vector<ASTNode*>& children) : type(type), children(children) {}

ASTNode::~ASTNode() {
    for(size_t i = 0; i < this->children.size(); ++i)
        delete this->children[i];
}

void ASTNode::print(std::ostream& os, size_t level) const {
    for(size_t i = 0; i < level; ++i)
        os << "    ";

    os << NODE_NAMES[static_cast<size_t>(this->type)] << std::endl;
    for(size_t i = 0; i < this->children.size(); ++i)
        this->children[i]->print(os, level+1);
}

std::ostream& operator<<(std::ostream& os, const ASTNode& node) {
    node.print(os);
    return os;
}