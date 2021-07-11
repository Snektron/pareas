#include "test/funccallastnodetempl.hpp"
#include "test/astgenerator.hpp"

FuncCallASTNodeTempl::FuncCallASTNodeTempl(const std::vector<DataType>& result_types, NodeType arg_list_type, NodeType arg_type) : 
    result_types(result_types), arg_list_type(arg_list_type), arg_type(arg_type) {

}

ASTNode* FuncCallASTNodeTempl::generate(NodeType node_type, ASTGenerator* generator) {
    auto data_type = generator->getValidDataType(this->result_types);
    std::pair<std::string, std::vector<DataType>> func_info = generator->findFunction(data_type);

    //In case a void call was selected, but no void functions are available
    if(func_info.first == "" && data_type == DataType::VOID) {
        std::vector<DataType> fallback_data_types;
        for(DataType d : this->result_types) {
            if(d != DataType::VOID)
                fallback_data_types.push_back(d);
        }
        data_type = generator->getValidDataType(fallback_data_types);
        func_info = generator->findFunction(data_type);
    }

    std::string func_name = func_info.first;
    std::vector<DataType> func_args_types = func_info.second;

    if(func_name == "" && data_type != DataType::VOID) { //No valid functions, generate a constant instead
        generator->pushDataType({data_type});
        ASTNode* result = generator->generate(NodeType::LIT_EXPR);
        generator->popDataType();
        return result;
    }

    std::vector<ASTNode*> func_args;
    for(DataType d : func_args_types) {
        generator->pushDataType({d});
        func_args.push_back(generator->generate(this->arg_type));
        generator->popDataType();
    }

    ASTNode* child = new ASTNode {
        .node_type = this->arg_list_type,
        .data_type = DataType::INVALID,
        .children = func_args,
        .node_data = 0
    };

    ASTNode* result = new ASTNode {
        .node_type = node_type,
        .data_type = data_type,
        .children = {child},
        .node_data = 0,
        .node_str = func_name
    };

    return result;
}

std::vector<DataType> FuncCallASTNodeTempl::getPossibleReturnTypes() {
    return this->result_types;
}