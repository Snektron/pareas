#ifndef _PAREAS_TEST_CONDASTNODETEMPL_HPP_
#define _PAREAS_TEST_CONDASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class CondASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> cond_nodes;
        std::vector<DataType> cond_types;
        std::vector<std::vector<NodeType>> child_types;
    public:
        CondASTNodeTempl(const std::vector<NodeType>&, const std::vector<DataType>&, const std::vector<std::vector<NodeType>>&);
        virtual ~CondASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif