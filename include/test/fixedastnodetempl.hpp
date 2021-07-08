#ifndef _PAREAS_TEST_FIXEDASTNODETEMPL_HPP_
#define _PAREAS_TEST_FIXEDASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class FixedASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<std::vector<NodeType>> child_types;
        std::vector<std::vector<DataType>> child_type_options;
    public:
        FixedASTNodeTempl(const std::vector<std::vector<NodeType>>&, const std::vector<std::vector<DataType>>&);
        virtual ~FixedASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif