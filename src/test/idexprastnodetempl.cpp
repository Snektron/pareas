#include "test/idexprastnodetempl.hpp"
#include "test/astgenerator.hpp"

IdExprASTNodeTempl::IdExprASTNodeTempl(const std::vector<DataType>& result_types) : 
    result_types(result_types) {

}

ASTNode* IdExprASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    auto data_type = generator->getValidDataType(this->result_types);

    std::string symbol = generator->findSymbol(data_type);
    if(symbol.size() == 0)
        return nullptr; //Forward to parent

    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = data_type,
        .children = {},
        .node_data = 0,
        .node_str = symbol
    };

    return result;
}

std::vector<DataType> IdExprASTNodeTempl::getPossibleReturnTypes() {
    return this->result_types;
}