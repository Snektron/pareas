#include "test/derefexprastnodetempl.hpp"
#include "test/astgenerator.hpp"

#include <iostream>

DerefExprASTNodeTempl::DerefExprASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types) : 
    node_types(node_types), result_types(result_types) {

}

ASTNode* DerefExprASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    auto data_type = generator->getValidDataType(this->result_types);

    generator->pushDataType({data_type == DataType::INT ? DataType::INT_REF : DataType::FLOAT_REF});
    auto child1_type = generator->chooseChildNode(this->node_types);
    ASTNode* child = generator->generate(child1_type);
    generator->popDataType();

    if(child == nullptr) { //No valid ids, just generate a constant
        generator->pushDataType({data_type});
        ASTNode* result = generator->generate(NodeType::LIT_EXPR);
        generator->popDataType();
        return result;
    }

    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = data_type,
        .children = {child},
        .node_data = 0
    };


    return result;
}

std::vector<DataType> DerefExprASTNodeTempl::getPossibleReturnTypes() {
    return this->result_types;
}