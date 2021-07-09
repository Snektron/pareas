#ifndef _PAREAS_TEST_ASSIGNEXPRASTNODETEMPL_HPP_
#define _PAREAS_TEST_ASSIGNEXPRASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class AssignExprASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> ref_types;
        std::vector<NodeType> node_types;
        std::vector<DataType> result_types;
        
    public:
        AssignExprASTNodeTempl(const std::vector<NodeType>& ref_node_types, const std::vector<NodeType>& node_types, const std::vector<DataType>& result_types);
        virtual ~AssignExprASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif