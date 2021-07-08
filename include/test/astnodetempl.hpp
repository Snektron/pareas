#ifndef _PAREAS_TEST_ASTNODETEMPL_HPP
#define _PAREAS_TEST_ASTNODETEMPL_HPP

#include "test/datatype.hpp"
#include "test/nodetype.hpp"
#include "test/astnode.hpp"

#include <vector>

class ASTGenerator;

class ASTNodeTempl {
    public:
        virtual ~ASTNodeTempl() = default;

        virtual ASTNode* generate(NodeType, ASTGenerator*) = 0;
        virtual std::vector<DataType> getPossibleReturnTypes() = 0;
};

#endif