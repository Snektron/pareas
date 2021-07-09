#ifndef _PAREAS_TEST_ASTGENERATOR_HPP_
#define _PAREAS_TEST_ASTGENERATOR_HPP_

#include "test/astnodetempl.hpp"
#include "test/nodetype.hpp"
#include "test/astnode.hpp"

#include <unordered_map>
#include <random>

class ASTGenerator {
    private:
        std::mt19937 rng;
        size_t target_width, target_height;
        uint32_t min_int_const, max_int_const;
        float min_flt_const, max_flt_const;
        size_t max_id_length;
        size_t max_stat_list_len, max_func_list_len, max_func_arg_list_len;

        std::unordered_map<NodeType, ASTNodeTempl*> node_options;

        size_t current_depth = 0;
        std::vector<size_t> current_widths;

        std::vector<std::vector<DataType>> datatype_stack;
        std::vector<std::unordered_map<std::string, DataType>> scopes;
        std::unordered_map<std::string, DataType> changed_scope;

        std::unordered_map<std::string, std::pair<DataType, std::vector<DataType>>> functab;

        DataType retval_type;

        void enterLayer();
        void exitLayer();

        size_t genRandomInt(size_t, size_t);
        std::string randomId();
    public:
        ASTGenerator(size_t, size_t, size_t, uint32_t, uint32_t, float, float, size_t, size_t, size_t, size_t);
        ~ASTGenerator();

        void add(NodeType, ASTNodeTempl*);

        ASTNode* generate(NodeType);

        inline void pushDataType(const std::vector<DataType>& types) {
            this->datatype_stack.push_back(types);
        }
        inline void popDataType() {
            this->datatype_stack.pop_back();
        }

        DataType getValidDataType(const std::vector<DataType>&);
        DataType pickRetType(const std::vector<DataType>&);
        NodeType chooseChildNode(const std::vector<NodeType>&);

        inline void enterScope() {
            this->scopes.push_back({});
            this->changed_scope.clear();
        }
        inline void exitScope() {
            this->scopes.pop_back();
            this->changed_scope.clear();
        }
        inline void commitScope() {
            this->scopes.back() = this->changed_scope;
        }

        inline void setRetvalType(DataType type) {
            this->retval_type = type;
        }
        inline DataType getRetvalType() {
            return this->retval_type;
        }

        std::string findSymbol(DataType);
        std::pair<std::string, std::vector<DataType>> findFunction(DataType);
        std::string makeSymbol(DataType);
        std::string makeFunction(DataType, const std::vector<DataType>&);

        uint32_t randomIntConst();
        float randomFloatConst();
        size_t randomStatementListLen(NodeType);
        size_t randomFuncArgListLen();
};

#endif