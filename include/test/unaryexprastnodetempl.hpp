#ifndef _PAREAS_TEST_UNARYEXPRASTNODETEMPL_HPP_
#define _PAREAS_TEST_UNARYEXPRASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class UnaryExprASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> node_types;
        std::vector<DataType> result_types;
        
    public:
        UnaryExprASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types);
        virtual ~UnaryExprASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif