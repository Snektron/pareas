#ifndef _PAREAS_TEST_FUNCDECLASTNODETEMPL_HPP_
#define _PAREAS_TEST_FUNCDECLASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class FuncDeclASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> child_types;
        NodeType arg_list_type;
        std::vector<DataType> retval_types;
    public:
        FuncDeclASTNodeTempl(const std::vector<NodeType>&, NodeType, const std::vector<DataType>&);
        virtual ~FuncDeclASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif