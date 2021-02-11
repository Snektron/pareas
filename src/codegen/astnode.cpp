#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"

#include <iostream>

const char* NODE_NAMES[] = {
    "invalid",
    "statement list",
    "empty statement",
    "function declaration",
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

const char* TYPE_NAMES[] = {
    "invalid",
    "void",
    "int",
    "float",
    "int_ref",
    "float_ref"
};

ASTNode::ASTNode(NodeType type) : type(type) {}
ASTNode::ASTNode(NodeType type, const std::vector<ASTNode*>& children) : type(type), children(children) {}
ASTNode::ASTNode(NodeType type, DataType return_type, uint32_t integer) : type(type), return_type(return_type), integer(integer) {}
ASTNode::ASTNode(NodeType type, DataType return_type, const std::vector<ASTNode*>& children) : type(type), return_type(return_type), children(children) {}

ASTNode::~ASTNode() {
    for(size_t i = 0; i < this->children.size(); ++i)
        delete this->children[i];
}

void ASTNode::print(std::ostream& os, size_t level) const {
    for(size_t i = 0; i < level; ++i)
        os << "    ";

    os << NODE_NAMES[static_cast<size_t>(this->type)] << " (";
    os << TYPE_NAMES[static_cast<size_t>(this->return_type)];

    switch(this->type) {
        case NodeType::LIT_EXPR:
            os << ", " << this->integer;
            break;
        default:
            break;
    }

    os << ")" << std::endl;

    for(size_t i = 0; i < this->children.size(); ++i)
        this->children[i]->print(os, level+1);
}


void ASTNode::resolveType() {
    for(size_t i = 0; i < this->children.size(); ++i)
        this->children[i]->resolveType();

    auto assert_type = [&](size_t child, const std::vector<DataType>& data_types) {
        for(size_t i = 0; i < data_types.size(); ++i) {
            if(this->children[child]->return_type == data_types[i])
                return;
        }
        this->print(std::cout);
        throw ParseException("Mismatched type on child ", child, " of node ",
                                NODE_NAMES[static_cast<size_t>(this->type)], ", got ",
                                TYPE_NAMES[static_cast<size_t>(this->children[child]->return_type)]);
    };

    auto assert_same = [&](size_t child1, size_t child2) {
        if(this->children[child1]->return_type != this->children[child2]->return_type)
            throw ParseException("Mismatched types for children ", child1, " and ", child2);
    };

    switch(this->type) {
        case NodeType::IF_STAT:
        case NodeType::IF_ELSE_STAT:
        case NodeType::WHILE_STAT:
            assert_type(0, {DataType::INT});
            break;
        case NodeType::ADD_EXPR:
        case NodeType::SUB_EXPR:
        case NodeType::MUL_EXPR:
        case NodeType::DIV_EXPR:
            assert_same(0, 1);
            assert_type(0, {DataType::INT, DataType::FLOAT});
            this->return_type = this->children[0]->return_type;
            break;
        case NodeType::MOD_EXPR:
        case NodeType::BITAND_EXPR:
        case NodeType::BITOR_EXPR:
        case NodeType::BITXOR_EXPR:
        case NodeType::LSHIFT_EXPR:
        case NodeType::RSHIFT_EXPR:
        case NodeType::URSHIFT_EXPR:
        case NodeType::LAND_EXPR:
        case NodeType::LOR_EXPR:
            assert_type(0, {DataType::INT});
            assert_type(1, {DataType::INT});
            this->return_type = DataType::INT;
            break;
        case NodeType::EQ_EXPR:
        case NodeType::NEQ_EXPR:
        case NodeType::LESS_EXPR:
        case NodeType::GREAT_EXPR:
        case NodeType::LESSEQ_EXPR:
        case NodeType::GREATEQ_EXPR:
            assert_same(0, 1);
            assert_type(0, {DataType::INT, DataType::FLOAT});
            this->return_type = DataType::INT;
            break;
        case NodeType::BITNOT_EXPR:
        case NodeType::LNOT_EXPR:
            assert_type(0, {DataType::INT});
            this->return_type = DataType::INT;
            break;
        case NodeType::NEG_EXPR:
            assert_type(0, {DataType::INT, DataType::FLOAT});
            this->return_type = this->children[0]->return_type;
            break;
        case NodeType::CAST_EXPR:
            assert_type(0, {DataType::INT, DataType::FLOAT});
            break;
        case NodeType::LIT_EXPR:
            break;
        default:
            this->return_type = DataType::INVALID;
            break;
    }
}

std::ostream& operator<<(std::ostream& os, const ASTNode& node) {
    node.print(os);
    return os;
}