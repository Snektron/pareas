#ifndef _PAREAS_TEST_CASTEXPRASTNODETEMPL_HPP_
#define _PAREAS_TEST_CASTEXPRASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class CastExprASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> node_types;
        std::vector<DataType> result_types;
        
    public:
        CastExprASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types);
        virtual ~CastExprASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif