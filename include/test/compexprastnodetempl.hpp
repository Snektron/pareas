#ifndef _PAREAS_TEST_COMPEXPRASTNODETEMPL_HPP_
#define _PAREAS_TEST_COMPEXPRASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class CompExprASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> node_types;
        std::vector<DataType> result_types;
        
    public:
        CompExprASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types);
        virtual ~CompExprASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif