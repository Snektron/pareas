#ifndef _PAREAS_TEST_FUNCCALLASTNODETEMPL_HPP_
#define _PAREAS_TEST_FUNCCALLASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class FuncCallASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<DataType> result_types;
        NodeType arg_list_type;
        NodeType arg_type;
    public:
        FuncCallASTNodeTempl(const std::vector<DataType>& result_types, NodeType arg_list_type, NodeType arg_type);
        virtual ~FuncCallASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif