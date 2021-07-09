#include "test/unaryexprastnodetempl.hpp"
#include "test/astgenerator.hpp"

UnaryExprASTNodeTempl::UnaryExprASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types) : 
    node_types(node_types), result_types(result_types) {

}

ASTNode* UnaryExprASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    auto data_type = generator->getValidDataType(this->result_types);

    generator->pushDataType({data_type});

    auto child1_type = generator->chooseChildNode(this->node_types);

    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = data_type,
        .children = {generator->generate(child1_type)},
        .node_data = 0
    };

    generator->popDataType();

    return result;
}

std::vector<DataType> UnaryExprASTNodeTempl::getPossibleReturnTypes() {
    return this->result_types;
}