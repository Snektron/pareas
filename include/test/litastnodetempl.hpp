#ifndef _PAREAS_TEST_LITASTNODETEMPL_HPP_
#define _PAREAS_TEST_LITASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class LitASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> node_types;
        std::vector<DataType> result_types;
    public:
        LitASTNodeTempl(const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types);
        virtual ~LitASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif