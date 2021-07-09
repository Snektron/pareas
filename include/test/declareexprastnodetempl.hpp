#ifndef _PAREAS_TEST_DECLAREEXPRASTNODETEMPL_HPP_
#define _PAREAS_TEST_DECLAREEXPRASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class DeclareExprASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<DataType> result_types;
    public:
        DeclareExprASTNodeTempl(const std::vector<DataType>& result_types);
        virtual ~DeclareExprASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif