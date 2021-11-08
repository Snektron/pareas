#include "converter/converter.hpp"
#include "codegen/astnode.hpp"
#include "codegen/symtab.hpp"

#include <iostream>
#include <cstring>
#include <iomanip>

const char* OPERATOR_NAMES[] = {
    "", // INVALID
    "", // STATEMENT_LIST
    "", // EMPTY_STAT
    "", // FUNC_DECL
    "", //FUNC_ARG
    "", //FUNC_ARG_LIST
    "", //EXPR_STAT
    "", // IF_STAT
    "", // IF_ELSE_STAT
    "", // WHILE_STAT
    "", // FUNC_CALL_EXPR
    "", //FUNC_CALL_ARG
    "", //FUNC_CALL_ARG_LIST
    "+", //ADD_EXPR
    "-", //SUB_EXPR
    "*", //MUL_EXPR
    "/", //DIV_EXPR
    "%", //MOD_EXPR
    "&", //BITAND_EXPR
    "|", //BITOR_EXPR
    "^", //BITXOR_EXPR
    "<<", //LSHIFT_EXPR
    ">>", //RSHIFT_EXPR
    ">>>", //URSHIFT_EXPR
    "&&", //LAND_EXPR
    "||", //LOR_EXPR
    "==", //EQ_EXPR
    "!=", //NEQ_EXPR
    "<", //LESS_EXPR
    ">", //GREAT_EXPR
    "<=", //LESSEQ_EXPR
    ">=", //GREATEQ_EXPR
    "~", //BITNOT_EXPR
    "!", //LNOT_EXPR
    "-", //NEG_EXPR
    "", //LIT_EXPR
    "", //CAST_EXPR
    "", //DEREF_EXPR
    "=", //ASSIGN_EXPR
    "", //DECL_EXPR
    "", //ID_EXPR
    "", //WHILE_DUMMY
    "", //FUNC_DECL_DUMMY
    "" //RETURN_STAT
};

const size_t OPERATOR_PRECEDENCE[] = {
    0, //INVALID
    0, //STATEMENT_LIST
    0, //EMPTY_STAT
    0, //FUNC_DECL
    0, //FUNC_ARG
    0, //FUNC_ARG_LIST
    0, //EXPR_STAT
    0, //IF_STAT
    0, //IF_ELSE_STAT
    0, //WHILE_STAT
    0, //FUNC_CALL_EXPR
    0, //FUNC_CALL_ARG
    0, //FUNC_CALL_ARG_LIST
    6, //ADD_EXPR
    6, //SUB_EXPR
    5, //MUL_EXPR
    5, //DIV_EXPR
    5, //MOD_EXPR
    11, //BITAND_EXPR
    11, //BITOR_EXPR
    11, //BITXOR_EXPR
    7, //LSHIFT_EXPR
    7, //RSHIFT_EXPR
    7, //URSHIFT_EXPR
    15, //LAND_EXPR
    14, //LOR_EXPR
    10, //EQ_EXPR
    10, //NEQ_EXPR
    9, //LESS_EXPR
    9, //GREAT_EXPR
    9, //LESSEQ_EXPR
    9, //GREATEQ_EXPR
    3, //BITNOT_EXPR
    3, //LNOT_EXPR
    3, //NEG_EXPR
    0, //LIT_EXPR
    2, //CAST_EXPR
    0, //DEREF_EXPR
    16, //ASSIGN_EXPR
    0, //DECL_EXPR
    0, //ID_EXPR
    0, //WHILE_DUMMY
    0, //FUNC_DECL_DUMMY
    0 //RETURN_STAT
};


namespace pareas {
    SourceConverter::SourceConverter(std::ostream& os, const SymbolTable* symtab) : os(os), symtab(symtab) {}

    void SourceConverter::convert(const ASTNode* node) {
        switch(node->getType()) {
            case NodeType::INVALID:
                break;
            case NodeType::STATEMENT_LIST:
                this->convertStatementList(node);
                break;
            case NodeType::EMPTY_STAT:
                this->printIndent();
                this->os << ";" << std::endl;
                break;
            case NodeType::FUNC_DECL:
                this->convertFunctionDecl(node);
                break;
            case NodeType::FUNC_ARG:
                this->convertArgDecl(node);
                break;
            case NodeType::FUNC_ARG_LIST:
                this->convertArgList(node);
                break;
            case NodeType::EXPR_STAT:
                this->printIndent();
                this->convert(node->getChildren()[0]);
                this->os << ";" << std::endl;
                break;
            case NodeType::IF_STAT:
                this->convertIf(node);
                break;
            case NodeType::IF_ELSE_STAT:
                this->convertIfElse(node);
                break;
            case NodeType::WHILE_STAT:
                this->convertWhile(node);
                break;
            case NodeType::FUNC_CALL_EXPR:
                this->convertFuncCall(node);
                break;
            case NodeType::FUNC_CALL_ARG:
                this->convert(node->getChildren()[0]);
                break;
            case NodeType::FUNC_CALL_ARG_LIST:
                this->convertArgList(node);
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
                this->convertBinaryOp(node);
                break;
            case NodeType::BITNOT_EXPR:
            case NodeType::LNOT_EXPR:
            case NodeType::NEG_EXPR:
                this->convertUnaryOp(node);
                break;
            case NodeType::LIT_EXPR:
                this->convertLiteral(node);
                break;
            case NodeType::CAST_EXPR:
                this->convertCast(node);
                break;
            case NodeType::DEREF_EXPR:
                this->convert(node->getChildren()[0]);
                break;
            case NodeType::ASSIGN_EXPR:
                this->convertAssignOp(node);
                break;
            case NodeType::DECL_EXPR:
                this->convertDecl(node);
                break;
            case NodeType::ID_EXPR:
                this->os << this->symtab->getVarName(node->getInteger());
                break;
            case NodeType::WHILE_DUMMY:
            case NodeType::FUNC_DECL_DUMMY:
                break;
            case NodeType::RETURN_STAT:
                this->convertReturn(node);
                break;
        }
    }

    void SourceConverter::convertStatementList(const ASTNode* node) {
        auto children = node->getChildren();

        if(children.size() == 0)
            return;

        for(size_t i = 0; i < children.size() - 1; ++i) {
            this->convert(children[i]);
        }

        if(children[children.size() - 1]->getType() != NodeType::EMPTY_STAT)
            this->convert(children[children.size() - 1]);
    }

    void SourceConverter::convertFunctionDecl(const ASTNode* node) {
        this->printIndent();
        this->os << "fn ";
        this->os << this->symtab->getFunctionName(node->getInteger());

        auto children = node->getChildren();
        this->convert(children[1]);

        this->os << " : ";
        this->printDataType(node->getResultingType());

        this->os << " {" << std::endl;
        this->enterScope();

        this->convert(children[2]);

        this->exitScope();
        this->printIndent();
        this->os << "}" << std::endl << std::endl;
    }

    void SourceConverter::convertArgList(const ASTNode* node) {
        this->os << "[";

        bool first = true;
        auto children = node->getChildren();
        for(const ASTNode* c : children) {
            if(first)
                first = false;
            else
                os << ", ";

            this->convert(c);
        }

        this->os << "]";
    }

    void SourceConverter::convertArgDecl(const ASTNode* node) {
        this->os << symtab->getVarName(node->getChildren()[0]->getInteger());
        this->os << ": ";
        this->printDataType(node->getResultingType());
    }

    void SourceConverter::convertBinaryOp(const ASTNode* node) {
        auto children = node->getChildren();
        auto node_type = static_cast<size_t>(node->getType());
        auto child1_type = static_cast<size_t>(children[0]->getType());
        auto child2_type = static_cast<size_t>(children[1]->getType());
        if(OPERATOR_PRECEDENCE[node_type] < OPERATOR_PRECEDENCE[child1_type])
            this->os << "(";

        this->convert(children[0]);

        if(OPERATOR_PRECEDENCE[node_type] < OPERATOR_PRECEDENCE[child1_type])
            this->os << ")";
        this->os << OPERATOR_NAMES[node_type];

        bool should_bracket = OPERATOR_PRECEDENCE[node_type] <= OPERATOR_PRECEDENCE[child2_type];
        if(should_bracket)
            this->os << "(";

        this->convert(children[1]);

        if(should_bracket)
            this->os << ")";
    }

    void SourceConverter::convertUnaryOp(const ASTNode* node) {
        auto children = node->getChildren();
        auto node_type = static_cast<size_t>(node->getType());
        auto child1_type = static_cast<size_t>(children[0]->getType());

        this->os << OPERATOR_NAMES[node_type];

        if(OPERATOR_PRECEDENCE[node_type] < OPERATOR_PRECEDENCE[child1_type])
            this->os << "(";

        this->convert(children[0]);

        if(OPERATOR_PRECEDENCE[node_type] < OPERATOR_PRECEDENCE[child1_type])
            this->os << ")";
    }

    void SourceConverter::convertAssignOp(const ASTNode* node) {
        auto children = node->getChildren();
        auto node_type = static_cast<size_t>(node->getType());
        auto child1_type = static_cast<size_t>(children[0]->getType());
        auto child2_type = static_cast<size_t>(children[1]->getType());
        if(OPERATOR_PRECEDENCE[node_type] <= OPERATOR_PRECEDENCE[child1_type])
            this->os << "(";

        this->convert(children[0]);

        if(OPERATOR_PRECEDENCE[node_type] <= OPERATOR_PRECEDENCE[child1_type])
            this->os << ")";
        this->os << OPERATOR_NAMES[node_type];

        bool should_bracket = OPERATOR_PRECEDENCE[node_type] < OPERATOR_PRECEDENCE[child2_type];
        if(should_bracket)
            this->os << "(";

        this->convert(children[1]);

        if(should_bracket)
            this->os << ")";
    }

    void SourceConverter::convertLiteral(const ASTNode* node) {
        uint32_t data = node->getInteger();

        if(node->getResultingType() == DataType::FLOAT) {
            float f;
            std::memcpy(&f, &data, sizeof(float));
            this->os << std::fixed << std::setprecision(3) << f;
        }
        else {
            this->os << data;
        }
    }

    void SourceConverter::convertCast(const ASTNode* node) {
        this->printDataType(node->getResultingType());
        this->os << "(";
        this->convert(node->getChildren()[0]);
        this->os << ")";
    }

    void SourceConverter::convertDecl(const ASTNode* node) {
        this->os << "var ";
        this->os << this->symtab->getVarName(node->getInteger());
        this->os << ": ";
        this->printDataType(node->getResultingType());
    }

    void SourceConverter::convertReturn(const ASTNode* node) {
        this->printIndent();
        this->os << "return";

        auto ret_type = node->getResultingType();
        if(ret_type == DataType::INT || ret_type == DataType::FLOAT) {
            this->os << " ";
            this->convert(node->getChildren()[0]);
        }
        this->os << ";" << std::endl;
    }

    void SourceConverter::convertIf(const ASTNode* node) {
        auto children = node->getChildren();

        this->printIndent();
        this->os << "if ";
        this->convert(children[0]);
        this->os << " {" << std::endl;

        this->enterScope();
        this->convert(children[1]);
        this->exitScope();
        this->printIndent();
        this->os << "}" << std::endl;
    }

    void SourceConverter::convertIfElse(const ASTNode* node) {
        auto children = node->getChildren();

        this->printIndent();
        this->os << "if ";
        this->convert(children[0]);
        this->os << " {" << std::endl;

        this->enterScope();
        this->convert(children[1]);
        this->exitScope();
        this->printIndent();
        this->os << "}" << std::endl;

        this->printIndent();
        this->os << "else {" << std::endl;
        this->enterScope();
        this->convert(children[2]);
        this->exitScope();
        this->printIndent();
        this->os << "}" << std::endl;
    }

    void SourceConverter::convertWhile(const ASTNode* node) {
        auto children = node->getChildren();

        this->printIndent();
        this->os << "while ";
        this->convert(children[1]);
        this->os << " {" << std::endl;

        this->enterScope();
        this->convert(children[2]);
        this->exitScope();
        this->printIndent();
        this->os << "}" << std::endl;
    }

    void SourceConverter::convertFuncCall(const ASTNode* node) {
        this->os << this->symtab->getFunctionName(node->getInteger());

        this->convert(node->getChildren()[0]);
    }

    void SourceConverter::printIndent() {
        for(size_t i = 0; i < this->indent; ++i) {
            this->os << "    ";
        }
    }

    void SourceConverter::printDataType(DataType d) {
        switch(d) {
            case DataType::INVALID:
                this->os << "invalid";
                break;
            case DataType::INT:
            case DataType::INT_REF:
                this->os << "int";
                break;
            case DataType::FLOAT:
            case DataType::FLOAT_REF:
                this->os << "float";
                break;
            case DataType::VOID:
                this->os << "void";
                break;
        }
    }
}
