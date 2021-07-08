#include "test/litastnodetempl.hpp"
#include "test/astgenerator.hpp"

#include <cstring>

LitASTNodeTempl::LitASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types) : 
    node_types(node_types), result_types(result_types) {

}

ASTNode* LitASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    auto data_type = generator->getValidDataType(this->result_types);

    uint32_t constant;
    if(data_type == DataType::FLOAT) {
        float c = generator->randomFloatConst();

        std::memcpy(&constant, &c, sizeof(uint32_t));
    }
    else {
        constant = generator->randomIntConst();
    }

    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = data_type,
        .children = {},
        .node_data = constant
    };

    return result;
}

std::vector<DataType> LitASTNodeTempl::getPossibleReturnTypes() {
    return this->result_types;
}