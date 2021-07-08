#include <iostream>
#include <ctime>

#include "test/astnodetempl.hpp"
#include "test/exprastnodetempl.hpp"
#include "test/litastnodetempl.hpp"
#include "test/fixedastnodetempl.hpp"

#include "test/astgenerator.hpp"

#include "test/defaulttreeprinter.hpp"
#include "test/sourcefileprinter.hpp"

const std::vector<NodeType> ALL_EXPRESSIONS = {NodeType::ADD_EXPR, NodeType::SUB_EXPR, NodeType::LIT_EXPR};

const std::vector<DataType> VALUE_TYPES = {DataType::INT, DataType::FLOAT};
const std::vector<DataType> INVALID_TYPE = {DataType::INVALID};

int main() {
    ASTGenerator generator(std::time(nullptr), 5, 10, 0, 100, 0, 100);

    generator.add(NodeType::EXPR_STAT, new FixedASTNodeTempl({ALL_EXPRESSIONS}, {VALUE_TYPES}));
    
    generator.add(NodeType::ADD_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    generator.add(NodeType::SUB_EXPR, new ExprASTNodeTempl(ALL_EXPRESSIONS, VALUE_TYPES));
    
    generator.add(NodeType::LIT_EXPR, new LitASTNodeTempl({}, VALUE_TYPES));

    ASTNode* root = generator.generate(NodeType::EXPR_STAT);

    TreePrinter* p = new SourceFilePrinter(std::cout);
    p->print(root);

    delete root;
    
    return 0;
}