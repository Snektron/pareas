#include "test/statementlistastnodetempl.hpp"
#include "test/astgenerator.hpp"
#include "test/astnode.hpp"

StatementListASTNodeTempl::StatementListASTNodeTempl(const std::vector<NodeType>& node_types) : 
    node_types(node_types) {

}

ASTNode* StatementListASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    std::vector<ASTNode*> children;
    size_t num_children = generator->randomStatementListLen(node_type);
    for(size_t i = 0; i < num_children; ++i) {
        auto child = generator->chooseChildNode(this->node_types);
        children.push_back(generator->generate(child));
    }
    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = DataType::INVALID,
        .children = children,
        .node_data = 0
    };

    return result;
}

std::vector<DataType> StatementListASTNodeTempl::getPossibleReturnTypes() {
    return {DataType::INVALID};
}