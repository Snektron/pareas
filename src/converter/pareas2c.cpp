/* Parses Pareas source files using the C++ lexer and parser in the codegen/
 * directory. Subsequently, dumps the AST as C source file. Pretty printer
 * loosely based on SourceFilePrinter in src/test directory.
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <memory>

#include <cassert>
#include <cstring>

#include "codegen/lexer.hpp"
#include "codegen/parser.hpp"
#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"
#include "codegen/depthtree.hpp"
#include "codegen/symtab.hpp"
#include "codegen/treeproperties.hpp"


const char* OPERATORS[] = {
    "",  // invalid
    "",  // statement list
    "",  // empty statement
    "",  // func decl
    "",  // func arg
    "",  // func arg list
    "",  // expr stat
    "if",
    "if",
    "while",
    "",  // func call
    "",  // func call arg
    "",  // func call arg list
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
    ">>",
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
    "",    // lit
    "@",   // cast
    "",    // deref
    "=",   // assignment
    " ",   // decl
    "",    // id
    "",    // while dummy
    "",    // func decl dummy
    ""     // return stat
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
    2,
    0,
    16,
    0,
    0,
    0,
    0,
    0
};




class CTreePrinter {
    private:
        std::ostream& os;
        SymbolTable &symtab;
        size_t indent = 0;

        bool noDeclare = false;

        void printIndent();
        void declareVariables(ASTNode *node);

        void printFuncDecl(ASTNode*);
        void printFuncArgList(ASTNode* node);
        void printStatementList(ASTNode* node);
        void printStatement(ASTNode* node);
        void printReturnStatement(ASTNode* node);
        void printIfElseStatement(ASTNode* node);
        void printWhileStatement(ASTNode* node);
        void printBinaryExpression(ASTNode* node);
        void printAssignExpression(ASTNode* node);
        void printUnaryExpression(ASTNode* node);
        void printCast(ASTNode* node);
        void printLiteral(ASTNode* node);
        void printDeclaration(ASTNode* node);
        void printCall(ASTNode* node);
        void printId(ASTNode* node);

        void printDataType(DataType datatype);

    public:
        CTreePrinter(std::ostream&, SymbolTable&);

        void print(ASTNode*);

};


CTreePrinter::CTreePrinter(std::ostream& os, SymbolTable& symtab)
    : os(os), symtab(symtab) { }


void CTreePrinter::print(ASTNode* node) {
#if 0
    /* Helpful when debugging. */
    os << "// " << NODE_NAMES[static_cast<size_t>(node->getType())] << std::endl;
#endif

    auto& children = node->getChildren();

    /* Determine whether we want to declare variables defined within
     * expression trees, before traversing the expression tree. There
     * are some exceptions. In particular, while and if statements
     * have specific handling (only consider the conditions, not the
     * clauses).
     */
    bool shouldDeclare = false;
    if(not this->noDeclare and
       (node->getType() != NodeType::STATEMENT_LIST and
        node->getType() != NodeType::EXPR_STAT and
        node->getType() != NodeType::FUNC_DECL and
        node->getType() != NodeType::FUNC_ARG_LIST and
        node->getType() != NodeType::FUNC_ARG and
        node->getType() != NodeType::IF_STAT and
        node->getType() != NodeType::IF_ELSE_STAT and
        node->getType() != NodeType::WHILE_STAT and
        node->getType() != NodeType::DECL_EXPR)) {
       shouldDeclare = true;
    }

    if (shouldDeclare) {
        declareVariables(node);
        this->noDeclare = true;
    }

    switch(node->getType()) {
        case NodeType::STATEMENT_LIST:
            this->printStatementList(node);
            break;
        case NodeType::EMPTY_STAT:
        case NodeType::EXPR_STAT:
            this->printStatement(node);
            break;
        case NodeType::FUNC_DECL:
            this->printFuncDecl(node);
            break;
        case NodeType::FUNC_ARG_LIST:
        case NodeType::FUNC_CALL_ARG_LIST:
            this->printFuncArgList(node);
            break;
        case NodeType::IF_STAT:
        case NodeType::IF_ELSE_STAT:
            this->printIfElseStatement(node);
            break;
        case NodeType::WHILE_STAT:
            this->printWhileStatement(node);
            break;
        case NodeType::FUNC_CALL_EXPR:
            this->printCall(node);
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
            this->printCast(node);
            break;
        case NodeType::DECL_EXPR:
            this->printDeclaration(node);
            break;
        case NodeType::ID_EXPR:
            this->printId(node);
            break;
        case NodeType::ASSIGN_EXPR:
            this->printAssignExpression(node);
            break;
        case NodeType::WHILE_DUMMY:
        case NodeType::FUNC_DECL_DUMMY:
            break;
        case NodeType::RETURN_STAT:
            this->printReturnStatement(node);
            break;
        case NodeType::DEREF_EXPR:
        case NodeType::FUNC_ARG:
        case NodeType::FUNC_CALL_ARG:
            this->print(children[0]);
            break;

        default:
            os << "// Unhandled node!" << std::endl;
            break;
    }

    if (shouldDeclare) {
        this->noDeclare = false;
    }
}

void CTreePrinter::printIndent() {
    for(size_t i = 0; i < this->indent; ++i)
        os << "    ";
}

void CTreePrinter::declareVariables(ASTNode* node) {
    if(node->getType() == NodeType::DECL_EXPR) {
        this->printIndent();
        this->printDeclaration(node);
        os << ";" << std::endl;
        return;
    }

    for(ASTNode* n : node->getChildren())
        declareVariables(n);
}


void CTreePrinter::printFuncDecl(ASTNode* node) {
    auto& children = node->getChildren();
    assert(children[0]->getType() == NodeType::FUNC_DECL_DUMMY);
    assert(children[1]->getType() == NodeType::FUNC_ARG_LIST);
    assert(children[2]->getType() == NodeType::STATEMENT_LIST);

    this->printDataType(node->getResultingType());
    os << " " << this->symtab.getFunctionName(node->getInteger()) << " ";

    this->print(children[1]);

    os << "{" << std::endl;

    ++this->indent;
    this->print(children[2]);
    if(node->getResultingType() == DataType::INT) {
        this->printIndent();
        os << "return 0;" << std::endl;
    }
    else if(node->getResultingType() == DataType::FLOAT) {
        this->printIndent();
        os << "return 0.0;" << std::endl;
    }
    --this->indent;
    os << "}" << std::endl;
}

void CTreePrinter::printFuncArgList(ASTNode* node) {
    this->os << "(";

    auto& children = node->getChildren();
    for(size_t i = 0; i < children.size(); ++i) {
        if(i > 0)
            this->os << ", ";
        this->print(children[i]);
    }

    this->os << ")";
}

void CTreePrinter::printStatementList(ASTNode* node) {
    for(ASTNode* n : node->getChildren()) {
        this->print(n);
    }
}

void CTreePrinter::printStatement(ASTNode* node) {
    this->printIndent();

    for(ASTNode* n : node->getChildren()) {
        this->print(n);
    }
    this->os << ";" << std::endl;
}

void CTreePrinter::printReturnStatement(ASTNode *node) {
    auto& children = node->getChildren();

    this->printIndent();
    os << "return";

    if(children.size() > 0) {
        os << " ";
        this->print(children[0]);
    }
    os << ";" << std::endl;
}

void CTreePrinter::printIfElseStatement(ASTNode* node) {
    auto& children = node->getChildren();

    /* Forward declare variables found in the condition of the if-statement */
    declareVariables(children[0]);

    this->printIndent();
    os << OPERATORS[static_cast<size_t>(node->getType())];
    os << "(";
    this->noDeclare = true;
    this->print(children[0]);
    this->noDeclare = false;
    os << ") {" << std::endl;

    ++this->indent;
    this->print(children[1]);
    --this->indent;
    this->printIndent();
    os << "}" << std::endl;

    if(node->getType() == NodeType::IF_ELSE_STAT) {
        this->printIndent();
        os << "else {" << std::endl;

        ++this->indent;
        this->print(children[2]);
        --this->indent;
        this->printIndent();
        os << "}" << std::endl;
    }
}

void CTreePrinter::printWhileStatement(ASTNode* node) {
    auto& children = node->getChildren();
    assert(children[0]->getType() == NodeType::WHILE_DUMMY);

    /* Forward declare variables found in the while-condition. */
    declareVariables(children[1]);

    this->printIndent();
    os << OPERATORS[static_cast<size_t>(node->getType())];
    os << "(";
    this->noDeclare = true;
    this->print(children[1]);
    this->noDeclare = false;
    os << ") {" << std::endl;

    ++this->indent;
    this->print(children[2]);
    --this->indent;
    this->printIndent();
    os << "}" << std::endl;
}

void CTreePrinter::printBinaryExpression(ASTNode* node) {
    auto& children = node->getChildren();

    if(PRECEDENCE[static_cast<size_t>(node->getType())] < PRECEDENCE[static_cast<size_t>(children[0]->getType())])
        os << "(";

    this->print(children[0]);

    if(PRECEDENCE[static_cast<size_t>(node->getType())] < PRECEDENCE[static_cast<size_t>(children[0]->getType())])
        os << ")";
    os << " " << OPERATORS[static_cast<size_t>(node->getType())] << " ";

    bool should_bracket = PRECEDENCE[static_cast<size_t>(node->getType())] <= PRECEDENCE[static_cast<size_t>(children[1]->getType())];
    if(should_bracket)
        os << "(";

    this->print(children[1]);

    if(should_bracket)
        os << ")";
}

void CTreePrinter::printAssignExpression(ASTNode* node) {
    auto& children = node->getChildren();

    if(PRECEDENCE[static_cast<size_t>(node->getType())] <= PRECEDENCE[static_cast<size_t>(children[0]->getType())])
        os << "(";

    this->print(children[0]);

    if(PRECEDENCE[static_cast<size_t>(node->getType())] <= PRECEDENCE[static_cast<size_t>(children[0]->getType())])
        os << ")";
    os << OPERATORS[static_cast<size_t>(node->getType())];

    if(PRECEDENCE[static_cast<size_t>(node->getType())] < PRECEDENCE[static_cast<size_t>(children[1]->getType())])
        os << "(";

    this->print(children[1]);

    if(PRECEDENCE[static_cast<size_t>(node->getType())] < PRECEDENCE[static_cast<size_t>(children[1]->getType())])
        os << ")";
}

void CTreePrinter::printUnaryExpression(ASTNode* node) {
    auto& children = node->getChildren();

    os << OPERATORS[static_cast<size_t>(node->getType())] << " ";
    if(PRECEDENCE[static_cast<size_t>(node->getType())] < PRECEDENCE[static_cast<size_t>(children[0]->getType())])
        os << "(";

    this->print(children[0]);

    if(PRECEDENCE[static_cast<size_t>(node->getType())] < PRECEDENCE[static_cast<size_t>(children[0]->getType())])
        os << ")";
}

void CTreePrinter::printCast(ASTNode* node) {
    auto& children = node->getChildren();

    os << "(";
    this->printDataType(node->getResultingType());
    os << ")(";
    if(PRECEDENCE[static_cast<size_t>(node->getType())] < PRECEDENCE[static_cast<size_t>(children[0]->getType())])
        os << "(";

    this->print(children[0]);

    if(PRECEDENCE[static_cast<size_t>(node->getType())] < PRECEDENCE[static_cast<size_t>(children[0]->getType())])
        os << ")";
    os << ")";
}

void CTreePrinter::printLiteral(ASTNode* node) {
    if(node->getResultingType() == DataType::FLOAT) {
        float f;
        uint32_t data = node->getInteger();
        std::memcpy(&f, &data, sizeof(uint32_t));
        os << std::fixed << std::setprecision(3) << f;
    }
    else {
        os << node->getInteger();
    }
}

void CTreePrinter::printDeclaration(ASTNode* node) {
    if (not noDeclare) {
        this->printDataType(value_of(node->getResultingType()));
        os << " ";
    }
    os << this->symtab.getVarName(node->getInteger()) << " "
       << OPERATORS[static_cast<size_t>(node->getType())];
}

void CTreePrinter::printCall(ASTNode* node) {
    auto& children = node->getChildren();

    os << this->symtab.getFunctionName(node->getInteger());
    this->print(children[0]);
}

void CTreePrinter::printId(ASTNode* node) {
    os << this->symtab.getVarName(node->getInteger());
}

void CTreePrinter::printDataType(DataType datatype) {
    switch(datatype) {
        case DataType::INT:
            os << "int";
            break;
        case DataType::FLOAT:
            os << "float";
            break;
        case DataType::VOID:
            os << "void";
            break;
        case DataType::INT_REF:
        case DataType::FLOAT_REF:
        case DataType::INVALID:
            os << "invalid";
            break;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "No input filename given" << std::endl;
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];

    std::ifstream input(filename);

    Lexer lexer(input);
    SymbolTable symtab;
    Parser parser(lexer, symtab);

    std::unique_ptr<ASTNode> node(parser.parse());

    CTreePrinter printer(std::cout, symtab);
    printer.print(node.get());

    return EXIT_SUCCESS;
}
