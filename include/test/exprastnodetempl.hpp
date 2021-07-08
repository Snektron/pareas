#ifndef _PAREAS_TEST_EXPRASTNODETEMPL_HPP_
#define _PAREAS_TEST_EXPRASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class ExprASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> node_types;
        std::vector<DataType> result_types;
        
    public:
        ExprASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types);
        virtual ~ExprASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif