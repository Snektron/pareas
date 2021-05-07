#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"

#include <iostream>

const char* NODE_NAMES[] = {
    "invalid",
    "statement list",
    "empty statement",
    "function declaration",
    "function argument",
    "function argument list",
    "expression statement",
    "if statement",
    "if-else statement",
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
    "dereference expression",
    "assignment expression",
    "declaration expression",
    "identifier expression",
    "while dummy",
    "function declaration dummy"
};

ASTNode::ASTNode(NodeType type) : type(type) {}
ASTNode::ASTNode(NodeType type, const std::vector<ASTNode*>& children) : type(type), children(children) {}
ASTNode::ASTNode(NodeType type, DataType return_type, uint32_t integer) : type(type), return_type(return_type), integer(integer) {}
ASTNode::ASTNode(NodeType type, DataType return_type, const std::vector<ASTNode*>& children) : type(type), return_type(return_type), children(children) {}
ASTNode::ASTNode(NodeType type, DataType return_type, uint32_t integer, const std::vector<ASTNode*>& children) : type(type), return_type(return_type), children(children), integer(integer) {}

ASTNode::~ASTNode() {
    for(size_t i = 0; i < this->children.size(); ++i)
        delete this->children[i];
}

size_t ASTNode::size() const {
    size_t result = 1;

    for(const auto* child : this->children) {
        result += child->size();
    }

    return result;
}

void ASTNode::print(std::ostream& os, size_t level) const {
    for(size_t i = 0; i < level; ++i)
        os << "    ";

    os << NODE_NAMES[static_cast<size_t>(this->type)] << " (";
    os << this->return_type;

    switch(this->type) {
        case NodeType::FUNC_DECL:
        case NodeType::LIT_EXPR:
        case NodeType::ID_EXPR:
        case NodeType::DECL_EXPR:
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
        for(size_t i = 0; i < data_types.size(); ++i) {
            if(this->children[child]->return_type == reference_of(data_types[i])) {
                //Create implicit dereference
                ASTNode* old_node = this->children[child];
                this->children[child] = new ASTNode(NodeType::DEREF_EXPR, data_types[i], {old_node});
                return;
            }
        }
        this->print(std::cout);
        throw ParseException("Mismatched type on child ", child, " of node ",
                                NODE_NAMES[static_cast<size_t>(this->type)], ", got ",
                                this->children[child]->return_type);
    };

    auto assert_same = [&](size_t child1, size_t child2) {
        if(this->children[child1]->return_type != this->children[child2]->return_type)
            throw ParseException("Mismatched types for children ", child1, " and ", child2);
    };

    auto assert_not_type = [&](size_t child1, DataType datatype) {
        if(this->children[child1]->return_type == datatype)
            throw ParseException("Invalid type for child ", child1, ": ", datatype);
    };

    auto assert_ref_of = [&](size_t child1, size_t child2) {
        if(this->children[child1]->return_type != reference_of(this->children[child2]->return_type))
            throw ParseException("Mismatched reference type for children ", child1, " and ", child2);
    };

    switch(this->type) {
        case NodeType::IF_STAT:
        case NodeType::IF_ELSE_STAT:
            assert_type(0, {DataType::INT});
            this->return_type = DataType::VOID;
            break;
        case NodeType::WHILE_STAT:
            assert_type(1, {DataType::INT});
            this->return_type = DataType::VOID;
            break;
        case NodeType::ADD_EXPR:
        case NodeType::SUB_EXPR:
        case NodeType::MUL_EXPR:
        case NodeType::DIV_EXPR:
            assert_type(0, {DataType::INT, DataType::FLOAT});
            assert_type(1, {DataType::INT, DataType::FLOAT});
            assert_same(0, 1);
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
            assert_type(0, {DataType::INT, DataType::FLOAT});
            assert_type(1, {DataType::INT, DataType::FLOAT});
            assert_same(0, 1);
            this->return_type = DataType::INT;
            break;
        case NodeType::BITNOT_EXPR:
        case NodeType::LNOT_EXPR:
            assert_type(0, {DataType::INT});
            this->return_type = DataType::INT;
            break;
        case NodeType::NEG_EXPR:
        case NodeType::FUNC_CALL_ARG:
            assert_type(0, {DataType::INT, DataType::FLOAT});
            this->return_type = this->children[0]->return_type;
            break;
        case NodeType::CAST_EXPR:
            assert_type(0, {DataType::INT, DataType::FLOAT});
            assert_not_type(0, this->return_type);
            break;
        case NodeType::ASSIGN_EXPR:
            assert_type(0, {DataType::INT_REF, DataType::FLOAT_REF});
            assert_type(1, {DataType::INT, DataType::FLOAT});
            assert_ref_of(0, 1);
            this->return_type = this->children[1]->return_type; //TODO: actually check this
            break;
        case NodeType::LIT_EXPR:
        case NodeType::ID_EXPR:
        case NodeType::DECL_EXPR:
            break;
        case NodeType::FUNC_ARG:
            assert_type(0, {DataType::INT_REF, DataType::FLOAT_REF});
            this->return_type = this->children[0]->return_type;
            break;
        case NodeType::FUNC_CALL_ARG_LIST: {
            size_t num_int_args = 0;
            for(size_t i = 0; i < this->children.size(); ++i) {
                assert_type(i, {DataType::INT, DataType::FLOAT});
                DataType d_type = this->children[i]->return_type;
                size_t arg_idx = d_type == DataType::INT_REF ? num_int_args++ : i - num_int_args;
                this->children[i]->integer = arg_idx;
            }
        }
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