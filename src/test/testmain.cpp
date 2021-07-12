#include <iostream>
#include <ctime>
#include <fstream>

#include "test/astnodetempl.hpp"
#include "test/castexprastnodetempl.hpp"
#include "test/compexprastnodetempl.hpp"
#include "test/exprastnodetempl.hpp"
#include "test/unaryexprastnodetempl.hpp"
#include "test/litastnodetempl.hpp"
#include "test/fixedastnodetempl.hpp"
#include "test/declareexprastnodetempl.hpp"
#include "test/assignexprastnodetempl.hpp"
#include "test/derefexprastnodetempl.hpp"
#include "test/idexprastnodetempl.hpp"
#include "test/statementlistastnodetempl.hpp"
#include "test/funcdeclastnodetempl.hpp"
#include "test/condastnodetempl.hpp"
#include "test/returnastnodetempl.hpp"
#include "test/funccallastnodetempl.hpp"

#include "test/astgenerator.hpp"
#include "test/treeproperties.hpp"

#include "test/defaulttreeprinter.hpp"
#include "test/sourcefileprinter.hpp"

const std::vector<NodeType> ALL_EXPRESSIONS = {NodeType::FUNC_CALL_EXPR,
                                                NodeType::ADD_EXPR, NodeType::SUB_EXPR, NodeType::MUL_EXPR, NodeType::DIV_EXPR,
                                                NodeType::MOD_EXPR, NodeType::BITAND_EXPR, NodeType::BITOR_EXPR, NodeType::BITXOR_EXPR,
                                                NodeType::LSHIFT_EXPR, NodeType::RSHIFT_EXPR, NodeType::URSHIFT_EXPR,
                                                NodeType::LAND_EXPR, NodeType::LOR_EXPR, NodeType::EQ_EXPR, NodeType::NEQ_EXPR,
                                                NodeType::LESS_EXPR, NodeType::GREAT_EXPR, NodeType::LESSEQ_EXPR, NodeType::GREATEQ_EXPR,
                                                NodeType::BITNOT_EXPR, NodeType::LNOT_EXPR, NodeType::NEG_EXPR,
                                                NodeType::CAST_EXPR, NodeType::DEREF_EXPR, NodeType::ASSIGN_EXPR, NodeType::LIT_EXPR};


const std::vector<NodeType> REF_EXPRESSIONS = {NodeType::DECL_EXPR, NodeType::ID_EXPR};

const std::vector<NodeType> ALL_STATEMENTS = {NodeType::EXPR_STAT, NodeType::IF_STAT, NodeType::IF_ELSE_STAT, NodeType::WHILE_STAT,
                                                NodeType::RETURN_STAT}; //, NodeType::EMPTY_STAT};

const std::vector<DataType> FUNC_RET_TYPES = {DataType::INT, DataType::FLOAT, DataType::VOID};
const std::vector<DataType> INVALID_TYPE = {DataType::INVALID};

struct Options {
    size_t seed;
    size_t tree_width = 10;
    size_t tree_height = 10;
    uint32_t int_min = 0;
    uint32_t int_max = 100;
    float flt_min = 0;
    float flt_max = 100;
    size_t max_id_len = 10;
    size_t max_stat_list_len = 10;
    size_t max_func_list_len = 10;
    size_t max_func_arg_list_len = 10;
    const char* output_filename = nullptr;
};

int main(int argc, char* argv[]) {
    if(argc < 5) {
        std::cerr << "No output filename given" << std::endl;
        return EXIT_FAILURE;
    }

    Options options;
    options.seed = std::time(nullptr);
    options.output_filename = argv[1];

    options.tree_width = std::strtoull(argv[2], nullptr, 0);
    options.max_stat_list_len = std::strtoull(argv[3], nullptr, 0);
    options.max_func_list_len = std::strtoull(argv[4], nullptr, 0);

    ASTGenerator generator(options.seed,
                                options.tree_width,
                                options.tree_height,
                                options.int_min,
                                options.int_max,
                                options.flt_min,
                                options.flt_max,
                                options.max_id_len,
                                options.max_stat_list_len,
                                options.max_func_list_len,
                                options.max_func_arg_list_len);

    generator.add(NodeType::STATEMENT_LIST, new StatementListASTNodeTempl(ALL_STATEMENTS));
    generator.add(NodeType::EMPTY_STAT, new FixedASTNodeTempl({}, {}, true));
    generator.add(NodeType::FUNC_DECL, new FuncDeclASTNodeTempl({NodeType::STATEMENT_LIST}, NodeType::FUNC_ARG_LIST, FUNC_RET_TYPES));
    generator.add(NodeType::FUNC_ARG, new UnaryExprASTNodeTempl({NodeType::DECL_EXPR}, REFERENCE_TYPES));
    generator.add(NodeType::FUNC_ARG_LIST, new StatementListASTNodeTempl({NodeType::FUNC_ARG}));
    generator.add(NodeType::EXPR_STAT, new FixedASTNodeTempl({ALL_EXPRESSIONS}, {FUNC_RET_TYPES}, true));
    generator.add(NodeType::IF_STAT, new CondASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}, {ALL_STATEMENTS}));
    generator.add(NodeType::IF_ELSE_STAT, new CondASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}, {ALL_STATEMENTS, ALL_STATEMENTS}));
    generator.add(NodeType::WHILE_STAT, new CondASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}, {ALL_STATEMENTS}));
    generator.add(NodeType::FUNC_CALL_EXPR, new FuncCallASTNodeTempl(FUNC_RET_TYPES, NodeType::FUNC_CALL_ARG_LIST, NodeType::FUNC_CALL_ARG));
    generator.add(NodeType::FUNC_CALL_ARG_LIST, new FixedASTNodeTempl({}, {}, false)); //Unused, integrated into func call expr
    generator.add(NodeType::FUNC_CALL_ARG, new UnaryExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::ADD_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::SUB_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::MUL_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::DIV_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::MOD_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::BITAND_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::BITOR_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::BITXOR_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::LSHIFT_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::RSHIFT_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::URSHIFT_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::LAND_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::LOR_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::EQ_EXPR, new CompExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::NEQ_EXPR, new CompExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::LESS_EXPR, new CompExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::GREAT_EXPR, new CompExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::LESSEQ_EXPR, new CompExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::GREATEQ_EXPR, new CompExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::BITNOT_EXPR, new UnaryExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::LNOT_EXPR, new UnaryExprASTNodeTempl(ALL_EXPRESSIONS, {DataType::INT}));
    generator.add(NodeType::NEG_EXPR, new UnaryExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::LIT_EXPR, new LitASTNodeTempl({}, VALUE_TYPES));
    generator.add(NodeType::CAST_EXPR, new CastExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::DEREF_EXPR, new DerefExprASTNodeTempl({NodeType::ID_EXPR}, VALUE_TYPES));
    generator.add(NodeType::ASSIGN_EXPR, new AssignExprASTNodeTempl(REF_EXPRESSIONS, ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::DECL_EXPR, new DeclareExprASTNodeTempl(REFERENCE_TYPES));
    generator.add(NodeType::ID_EXPR, new IdExprASTNodeTempl(REFERENCE_TYPES));
    generator.add(NodeType::WHILE_DUMMY, new FixedASTNodeTempl({}, {}, false));
    generator.add(NodeType::FUNC_DECL_DUMMY, new FixedASTNodeTempl({}, {}, false));
    generator.add(NodeType::FUNC_DECL_LIST, new StatementListASTNodeTempl({NodeType::FUNC_DECL}));
    generator.add(NodeType::RETURN_STAT, new ReturnASTNodeTempl(ALL_EXPRESSIONS));


    generator.enterScope();
    generator.pushDataType({DataType::INVALID});
    ASTNode* root = generator.generate(NodeType::FUNC_DECL_LIST);

    // ASTNode* op3 = new ASTNode{NodeType::LIT_EXPR, DataType::INT, {}, 2};
    // ASTNode* op1 = new ASTNode{NodeType::LIT_EXPR, DataType::INT, {}, 1};
    // ASTNode* op2 = new ASTNode{NodeType::NEG_EXPR, DataType::INT, {op3}};
    // ASTNode* root = new ASTNode{NodeType::LESS_EXPR, DataType::INT, {op1, op2}};

    TreeProperties props(root);
    std::cout << "Generated tree with properties:" << std::endl
        << "\tseed: " << options.seed << std::endl
        << "\tdepth: " << props.getDepth() << std::endl 
        << "\twidth: " << props.getWidth() << std::endl
        << "\tfunctions: " << props.getFunctions() << std::endl
        << "\tmax function nodes: " << props.getMaxFuncLen() << std::endl;

    std::ofstream output(options.output_filename);
    if(!output) {
        std::cerr << "Failed to open output file " << options.output_filename << std::endl;
        return EXIT_FAILURE;
    }
    TreePrinter* p = new SourceFilePrinter(output);
    p->print(root);

    delete root;
    
    return 0;
}