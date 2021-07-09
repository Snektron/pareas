#include "test/declareexprastnodetempl.hpp"
#include "test/astgenerator.hpp"

DeclareExprASTNodeTempl::DeclareExprASTNodeTempl(const std::vector<DataType>& result_types) : 
    result_types(result_types) {

}

ASTNode* DeclareExprASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    auto data_type = generator->getValidDataType(this->result_types);

    std::string symbol = generator->makeSymbol(data_type);

    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = data_type,
        .children = {},
        .node_data = 0,
        .node_str = symbol
    };

    return result;
}

std::vector<DataType> DeclareExprASTNodeTempl::getPossibleReturnTypes() {
    return this->result_types;
}