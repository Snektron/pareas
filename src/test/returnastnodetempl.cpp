#include "test/returnastnodetempl.hpp"
#include "test/astgenerator.hpp"
#include "test/astnode.hpp"

ReturnASTNodeTempl::ReturnASTNodeTempl(const std::vector<NodeType>& expr_nodes)
    : expr_nodes(expr_nodes)
{

}

ASTNode* ReturnASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    std::vector<ASTNode*> children;

    DataType data_type = generator->getRetvalType();
    if(data_type != DataType::VOID) {
        generator->pushDataType({data_type});
        auto ret_type = generator->chooseChildNode(this->expr_nodes);
        ASTNode* child = generator->generate(ret_type);
        generator->popDataType();
        generator->commitScope();

        ASTNode* result = new ASTNode {
            .node_type = node_type,
            .data_type = DataType::INVALID,
            .children = {child},
            .node_data = 0
        };

        return result;
    }
    else {
        return new ASTNode {
            .node_type = node_type,
            .data_type = DataType::INVALID,
            .children = {},
            .node_data = 0
        };
    }
}

std::vector<DataType> ReturnASTNodeTempl::getPossibleReturnTypes() {
    return {DataType::INVALID};
}