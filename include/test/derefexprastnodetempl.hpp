#ifndef _PAREAS_TEST_DEREFEXPRASTNODETEMPL_HPP_
#define _PAREAS_TEST_DEREFEXPRASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class DerefExprASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> node_types;
        std::vector<DataType> result_types;
        
    public:
        DerefExprASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types);
        virtual ~DerefExprASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif