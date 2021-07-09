#ifndef _PAREAS_STATEMENTLISTASTNODETEMPL_HPP_
#define _PAREAS_STATEMENTLISTASTNODETEMPL_HPP_

#include "test/astnodetempl.hpp"

class StatementListASTNodeTempl : public ASTNodeTempl {
    private:
        std::vector<NodeType> node_types;
    public:
        StatementListASTNodeTempl(const std::vector<NodeType>& node_types);
        virtual ~StatementListASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*);
        virtual std::vector<DataType> getPossibleReturnTypes();
};

#endif