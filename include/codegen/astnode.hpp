#ifndef _PAREAS_CODEGEN_ASTNODE_HPP
#define _PAREAS_CODEGEN_ASTNODE_HPP

#include <vector>
#include <iosfwd>
#include <cstddef>
#include <cstdint>

#include "codegen/datatype.hpp"

enum class NodeType {
    INVALID,
    STATEMENT_LIST,
    EMPTY_STAT,
    FUNC_DECL,
    FUNC_ARG,
    FUNC_ARG_LIST,
    EXPR_STAT,
    IF_STAT,
    IF_ELSE_STAT,
    WHILE_STAT,
    FUNC_CALL_EXPR,
    FUNC_CALL_ARG,
    FUNC_CALL_ARG_LIST,
    ADD_EXPR,
    SUB_EXPR,
    MUL_EXPR,
    DIV_EXPR,
    MOD_EXPR,
    BITAND_EXPR,
    BITOR_EXPR,
    BITXOR_EXPR,
    LSHIFT_EXPR,
    RSHIFT_EXPR,
    URSHIFT_EXPR,
    LAND_EXPR,
    LOR_EXPR,
    EQ_EXPR,
    NEQ_EXPR,
    LESS_EXPR,
    GREAT_EXPR,
    LESSEQ_EXPR,
    GREATEQ_EXPR,
    BITNOT_EXPR,
    LNOT_EXPR,
    NEG_EXPR,
    LIT_EXPR,
    CAST_EXPR,
    DEREF_EXPR,
    ASSIGN_EXPR,
    DECL_EXPR,
    ID_EXPR,
    WHILE_DUMMY,
    FUNC_DECL_DUMMY,
    RETURN_STAT
};

extern const char* NODE_NAMES[];

class ASTNode {
    private:
        NodeType type;
        DataType return_type = DataType::INVALID;
        std::vector<ASTNode*> children;

        uint32_t integer = 0;
    public:
        ASTNode(NodeType);
        ASTNode(NodeType, const std::vector<ASTNode*>&);
        ASTNode(NodeType, DataType, const std::vector<ASTNode*>&);
        ASTNode(NodeType, DataType, uint32_t);
        ASTNode(NodeType, DataType, uint32_t, const std::vector<ASTNode*>&);
        ~ASTNode();

        inline NodeType getType() const {
            return this->type;
        }
        inline DataType getResultingType() const {
            return this->return_type;
        }
        inline const std::vector<ASTNode*>& getChildren() const {
            return this->children;
        }
        inline const uint32_t getInteger() const {
            return this->integer;
        }

        size_t size() const;

        void print(std::ostream&, size_t  = 0) const;
        void resolveType();
};

std::ostream& operator<<(std::ostream&, const ASTNode&);

#endif
