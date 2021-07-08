#include "test/sourcefileprinter.hpp"
#include "test/astnode.hpp"
#include "test/datatype.hpp"
#include "test/nodetype.hpp"

#include <iostream>
#include <cstring>

const char* OPERATORS[] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "+",
    "-",
    "*",
    "/",
    "%",
    "&",
    "|",
    "^",
    "<<",
    ">>",
    ">>>",
    "&&",
    "||",
    "==",
    "!=",
    "<",
    ">",
    "<=",
    ">=",
    "~",
    "!",
    "-",
    "",
    "",
    "",
    "=",
    "",
    "",
    "",
    "",
    "",
    ""
};

const size_t PRECEDENCE[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    6,
    6,
    5,
    5,
    5,
    11,
    11,
    11,
    7,
    7,
    7,
    14,
    14,
    10,
    10,
    9,
    9,
    9,
    9,
    3,
    3,
    3,
    0,
    0,
    0,
    16,
    0,
    0,
    0,
    0,
    0,
    0
};

SourceFilePrinter::SourceFilePrinter(std::ostream& os) : os(os) {}

void SourceFilePrinter::print(ASTNode* node) {
    switch(node->node_type) {
        case NodeType::STATEMENT_LIST:
        case NodeType::FUNC_DECL_LIST:
            this->printStatementList(node);
            break;
        case NodeType::EMPTY_STAT:
        case NodeType::EXPR_STAT:
            this->printStatement(node);
            break;
        case NodeType::FUNC_DECL:
            break;
        case NodeType::FUNC_ARG:
        case NodeType::FUNC_ARG_LIST:
            break;
        case NodeType::IF_STAT:
        case NodeType::IF_ELSE_STAT:
            break;
        case NodeType::WHILE_STAT:
            break;
        case NodeType::FUNC_CALL_EXPR:
            break;
        case NodeType::FUNC_CALL_ARG:
        case NodeType::FUNC_CALL_ARG_LIST:
            break;
        case NodeType::ADD_EXPR:
        case NodeType::SUB_EXPR:
        case NodeType::MUL_EXPR:
        case NodeType::DIV_EXPR:
        case NodeType::MOD_EXPR:
        case NodeType::BITAND_EXPR:
        case NodeType::BITOR_EXPR:
        case NodeType::BITXOR_EXPR:
        case NodeType::LSHIFT_EXPR:
        case NodeType::RSHIFT_EXPR:
        case NodeType::URSHIFT_EXPR:
        case NodeType::LAND_EXPR:
        case NodeType::LOR_EXPR:
        case NodeType::EQ_EXPR:
        case NodeType::NEQ_EXPR:
        case NodeType::LESS_EXPR:
        case NodeType::GREAT_EXPR:
        case NodeType::LESSEQ_EXPR:
        case NodeType::GREATEQ_EXPR:
        case NodeType::ASSIGN_EXPR:
            this->printBinaryExpression(node);
            break;
        case NodeType::BITNOT_EXPR:
        case NodeType::LNOT_EXPR:
        case NodeType::NEG_EXPR:
            this->printUnaryExpression(node);
            break;
        case NodeType::LIT_EXPR:
            this->printLiteral(node);
            break;
        case NodeType::CAST_EXPR:
            break;
        case NodeType::DEREF_EXPR:
            break;
        case NodeType::DECL_EXPR:
            break;
        case NodeType::ID_EXPR:
            break;
        case NodeType::WHILE_DUMMY:
        case NodeType::FUNC_DECL_DUMMY:
            break;
        case NodeType::RETURN_STAT:
            break;
    }
}

void SourceFilePrinter::printIndent() {
    for(size_t i = 0; i < this->indent; ++i) {
        this->os << "    ";
    }
}

void SourceFilePrinter::printStatementList(ASTNode* node) {
    for(ASTNode* n : node->children) {
        this->print(n);
    }
}

void SourceFilePrinter::printStatement(ASTNode* node) {
    this->printIndent();

    for(ASTNode* n : node->children) {
        this->print(n);
    }
    this->os << ";" << std::endl;
}

void SourceFilePrinter::printBinaryExpression(ASTNode* node) {
    if(PRECEDENCE[static_cast<size_t>(node->node_type)] <= PRECEDENCE[static_cast<size_t>(node->children[0]->node_type)])
        this->os << "(";

    this->print(node->children[0]);

    if(PRECEDENCE[static_cast<size_t>(node->node_type)] <= PRECEDENCE[static_cast<size_t>(node->children[0]->node_type)])
        this->os << ")";
    this->os << OPERATORS[static_cast<size_t>(node->node_type)];

    if(PRECEDENCE[static_cast<size_t>(node->node_type)] < PRECEDENCE[static_cast<size_t>(node->children[1]->node_type)])
        this->os << "(";

    this->print(node->children[1]);

    if(PRECEDENCE[static_cast<size_t>(node->node_type)] < PRECEDENCE[static_cast<size_t>(node->children[1]->node_type)])
        this->os << ")";
}

void SourceFilePrinter::printUnaryExpression(ASTNode* node) {
    this->os << OPERATORS[static_cast<size_t>(node->node_type)];
    if(PRECEDENCE[static_cast<size_t>(node->node_type)] < PRECEDENCE[static_cast<size_t>(node->children[0]->node_type)])
        this->os << "(";

    this->print(node->children[0]);

    if(PRECEDENCE[static_cast<size_t>(node->node_type)] < PRECEDENCE[static_cast<size_t>(node->children[0]->node_type)])
        this->os << ")";
}

void SourceFilePrinter::printLiteral(ASTNode* node) {
    if(node->data_type == DataType::FLOAT) {
        float f;
        std::memcpy(&f, &node->node_data, sizeof(uint32_t));
        os << f;
    }
    else {
        os << node->node_data;
    }
}