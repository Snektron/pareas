#ifndef _PAREAS_TEST_RETURNASTNODETEMPL_HPP_
#define _PAREAS_TEST_RETURNASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class ReturnASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> expr_nodes;
        
    public:
        ReturnASTNodeTempl(const std::vector<NodeType>&);
        virtual ~ReturnASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif