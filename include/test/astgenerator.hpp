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

        std::unordered_map<NodeType, ASTNodeTempl*> node_options;

        size_t current_depth = 0;
        std::vector<size_t> current_widths;

        std::vector<std::vector<DataType>> datatype_stack;

        void enterLayer();
        void exitLayer();

        size_t genRandomInt(size_t, size_t);
    public:
        ASTGenerator(size_t, size_t, size_t, uint32_t, uint32_t, float, float);
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
        NodeType chooseChildNode(const std::vector<NodeType>&);

        uint32_t randomIntConst();
        float randomFloatConst();
};

#endif