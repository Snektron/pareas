#ifndef _PAREAS_TEST_IDEXPRASTNODETEMPL_HPP_
#define _PAREAS_TEST_IDEXPRASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class IdExprASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<DataType> result_types;
    public:
        IdExprASTNodeTempl(const std::vector<DataType>& result_types);
        virtual ~IdExprASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif