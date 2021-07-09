#include "test/condastnodetempl.hpp"
#include "test/astgenerator.hpp"
#include "test/astnode.hpp"

CondASTNodeTempl::CondASTNodeTempl(const std::vector<NodeType>& cond_nodes, const std::vector<DataType>& cond_types, const std::vector<std::vector<NodeType>>& child_nodes)
    : cond_nodes(cond_nodes), cond_types(cond_types), child_types(child_nodes)
{

}

ASTNode* CondASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    std::vector<ASTNode*> children;
    generator->enterScope();
    generator->pushDataType(this->cond_types);
    auto cond_type = generator->chooseChildNode(this->cond_nodes);
    children.push_back(generator->generate(cond_type));
    generator->popDataType();
    generator->commitScope();

    for(size_t i = 0; i < this->child_types.size(); ++i) {
        generator->enterScope();
        auto child = generator->chooseChildNode(this->child_types[i]);
        children.push_back(generator->generate(child));
        generator->exitScope();
    }
    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = DataType::INVALID,
        .children = children,
        .node_data = 0
    };

    generator->exitScope();

    return result;
}

std::vector<DataType> CondASTNodeTempl::getPossibleReturnTypes() {
    return {DataType::INVALID};
}