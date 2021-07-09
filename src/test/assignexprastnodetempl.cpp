#include "test/assignexprastnodetempl.hpp"
#include "test/astgenerator.hpp"

AssignExprASTNodeTempl::AssignExprASTNodeTempl(const std::vector<NodeType>& ref_types, 
        const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types) : 
    ref_types(ref_types), node_types(node_types), result_types(result_types) {

}

ASTNode* AssignExprASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    auto data_type = generator->getValidDataType(this->result_types);

    generator->pushDataType({ref_of(data_type)});
    auto child1_type = generator->chooseChildNode(this->ref_types);
    ASTNode* first_child = generator->generate(child1_type);
    if(first_child == nullptr)
        first_child = generator->generate(NodeType::DECL_EXPR);
    generator->popDataType();

    generator->pushDataType({data_type});
    auto child2_type = generator->chooseChildNode(this->node_types);

    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = data_type,
        .children = {first_child, generator->generate(child2_type)},
        .node_data = 0
    };

    generator->popDataType();

    return result;
}

std::vector<DataType> AssignExprASTNodeTempl::getPossibleReturnTypes() {
    return this->result_types;
}