#include "test/fixedastnodetempl.hpp"
#include "test/astgenerator.hpp"
#include "test/astnode.hpp"

FixedASTNodeTempl::FixedASTNodeTempl(const std::vector<std::vector<NodeType>>& node_types, const std::vector<std::vector<DataType>>& result_types) : 
    child_types(node_types), child_type_options(result_types) {

}

ASTNode* FixedASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    std::vector<ASTNode*> children;
    for(size_t i = 0; i < this->child_types.size(); ++i) {
        generator->pushDataType(this->child_type_options[i]);

        auto child = generator->chooseChildNode(this->child_types[i]);
        children.push_back(generator->generate(child));

        generator->popDataType();
    }
    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = DataType::INVALID,
        .children = children,
        .node_data = 0
    };

    return result;
}

std::vector<DataType> FixedASTNodeTempl::getPossibleReturnTypes() {
    return {DataType::INVALID};
}