#include "test/defaulttreeprinter.hpp"

#include "test/astnode.hpp"

#include <iostream>
#include <cstring>

const char* NODE_NAMES[] = {
    "statement list",
    "empty statement",
    "function declaration",
    "function argument",
    "function argument list",
    "expression statement",
    "if statement",
    "if else statement",
    "while statement",
    "function call expression",
    "function call argument",
    "function call argument list",
    "add expression",
    "sub expression",
    "mul expression",
    "div expression",
    "mod expression",
    "bitwise and expression",
    "bitwise or expression",
    "bitwise xor expression",
    "left shift expression",
    "right shift expression",
    "unsigned right shift expression",
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
    "negation expression",
    "literal",
    "cast expression",
    "dereference expression",
    "assignment expression",
    "declaration expression",
    "identifier expression",
    "while dummy node",
    "function declaration dummy",
    "return statement",

    "statement list"
};

const char* TYPE_NAMES[] = {
    "int",
    "float",
    "int ref",
    "float ref",
    "void",
    "invalid"
};

DefaultTreePrinter::DefaultTreePrinter(std::ostream& os) : os(os) {}

void DefaultTreePrinter::printIndent() {
    for(size_t i = 0; i < this->indent; ++i) {
        this->os << "    ";
    }
}

void DefaultTreePrinter::print(ASTNode* root) {
    this->printIndent();
    this->os << NODE_NAMES[static_cast<size_t>(root->node_type)]
        << " (" << TYPE_NAMES[static_cast<size_t>(root->data_type)];


    if(root->node_type == NodeType::LIT_EXPR) {
        os << ", ";
        if(root->data_type == DataType::FLOAT) {
            float f;
            std::memcpy(&f, &root->node_data, sizeof(uint32_t));
            os << f;
        }
        else {
            os << root->node_data;
        }
    }
    this->os << ")" << std::endl;

    ++this->indent;

    for(ASTNode* node : root->children) {
        this->print(node);
    }

    --this->indent;
}