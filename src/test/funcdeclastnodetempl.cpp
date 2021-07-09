#include "test/funcdeclastnodetempl.hpp"
#include "test/astgenerator.hpp"
#include "test/astnode.hpp"

FuncDeclASTNodeTempl::FuncDeclASTNodeTempl(const std::vector<NodeType>& node_types, NodeType arg_list_type, const std::vector<DataType>& result_types) : 
    child_types(node_types), arg_list_type(arg_list_type), retval_types(result_types) {

}

ASTNode* FuncDeclASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    DataType retval_type = generator->pickRetType(this->retval_types);

    generator->enterScope();
    generator->pushDataType(REFERENCE_TYPES);
    ASTNode* arg_list = generator->generate(this->arg_list_type);
    generator->popDataType();
    generator->commitScope();

    std::vector<DataType> arg_types;
    for(ASTNode* arg : arg_list->children) {
        arg_types.push_back(value_of(arg->data_type));
    }

    std::string func_name = generator->makeFunction(retval_type, arg_types);
    generator->setRetvalType(retval_type);

    auto child_type = generator->chooseChildNode(this->child_types);
    ASTNode* child = generator->generate(child_type);

    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = retval_type,
        .children = {generator->generate(NodeType::FUNC_DECL_DUMMY), arg_list, child},
        .node_data = 0,
        .node_str = func_name
    };

    generator->exitScope();

    return result;
}

std::vector<DataType> FuncDeclASTNodeTempl::getPossibleReturnTypes() {
    return {DataType::INVALID};
}