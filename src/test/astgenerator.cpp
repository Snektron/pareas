#include "test/astgenerator.hpp"

#include <iostream>
#include <algorithm>

const size_t DEPTH_FACTORS[] = { //Per node estimate of depth expansion of the tree
    2, // STATEMENT_LIST
    0, // EMPTY_STAT
    3, // FUNC_DECL
    1, // FUNC_ARG
    2, // FUNC_ARG_LIST
    2, // EXPR_STAT
    3, // IF_STAT
    3, // IF_ELSE_STAT
    3, // WHILE_STAT
    3, // FUNC_CALL_EXPR
    1, // FUNC_CALL_ARG
    2, // FUNC_CALL_ARG_LIST
    1, // ADD_EXPR
    1, // SUB_EXPR
    1, // MUL_EXPR
    1, // DIV_EXPR
    1, // MOD_EXPR
    1, // BITAND_EXPR
    1, // BITOR_EXPR
    1, // BITXOR_EXPR
    1, // LSHIFT_EXPR
    1, // RSHIFT_EXPR
    1, // URSHIFT_EXPR
    1, // LAND_EXPR
    1, // LOR_EXPR
    1, // EQ_EXPR
    1, // NEQ_EXPR
    1, // LESS_EXPR
    1, // GREAT_EXPR
    1, // LESSEQ_EXPR
    1, // GREATEQ_EXPR
    1, // BITNOT_EXPR
    1, // LNOT_EXPR
    1, // NEG_EXPR
    0, // LIT_EXPR
    1, // CAST_EXPR
    1, // DEREF_EXPR
    1, // ASSIGN_EXPR
    0, // DECL_EXPR
    0, // ID_EXPR
    0, // WHILE_DUMMY
    0, // FUNC_DECL_DUMMY
    1, // RETURN_STAT

    5, // FUNC_DECL_LIST
};

ASTGenerator::ASTGenerator(size_t seed, size_t target_width, size_t target_height,
        uint32_t min_int_const, uint32_t max_int_const, float min_flt_const, float max_flt_const) : 
    rng(seed), target_width(target_width), target_height(target_height),
    min_int_const(min_int_const), max_int_const(max_int_const), min_flt_const(min_flt_const), max_flt_const(max_flt_const) {}

ASTGenerator::~ASTGenerator() {
    for(auto& it : this->node_options) {
        delete it.second;
    }
}

void ASTGenerator::add(NodeType type, ASTNodeTempl* templ) {
    this->node_options[type] = templ;
}

ASTNode* ASTGenerator::generate(NodeType root_type) {
    this->enterLayer();

    ASTNodeTempl* templ = this->node_options[root_type];
    ASTNode* result = templ->generate(root_type, this);

    this->exitLayer();

    return result;
}

void ASTGenerator::enterLayer() {
    ++this->current_depth;

    if(this->current_widths.size() < this->current_depth) {
        this->current_widths.push_back(1);
    }
    else {
        ++this->current_widths[this->current_depth - 1];
    }
}

void ASTGenerator::exitLayer() {
    --this->current_depth;
}

DataType ASTGenerator::getValidDataType(const std::vector<DataType>& valid_options) {
    std::vector<DataType> options;

    auto& stack_data = this->datatype_stack.back();

    std::set_intersection(stack_data.begin(), stack_data.end(), valid_options.begin(), valid_options.end(), std::back_inserter(options));

    return options[this->genRandomInt(0, options.size())];
}

size_t ASTGenerator::genRandomInt(size_t min, size_t max) {
    std::uniform_int_distribution<size_t> distrib(min, max-1);

    return distrib(this->rng);
}

uint32_t ASTGenerator::randomIntConst() {
    std::uniform_int_distribution<uint32_t> distrib(this->min_int_const, this->max_int_const);

    return distrib(this->rng);
}

float ASTGenerator::randomFloatConst() {
    std::uniform_real_distribution<float> distrib(this->min_flt_const, this->max_flt_const);

    return distrib(this->rng);
}

NodeType ASTGenerator::chooseChildNode(const std::vector<NodeType>& all_options) {
    std::vector<NodeType> options;

    for(NodeType n : all_options) {
        ASTNodeTempl* templ = this->node_options[n];

        std::vector<DataType> type_options;
        auto type_stack = this->datatype_stack.back();
        auto node_type_options = templ->getPossibleReturnTypes();
        std::set_intersection(type_stack.begin(), type_stack.end(), node_type_options.begin(), node_type_options.end(), std::back_inserter(type_options));
        if(type_options.size() > 0) {
            options.push_back(n);
        }
    }
    
    if(this->current_depth >= this->target_height) {

        size_t min_depth_factor = std::numeric_limits<size_t>::max();
        for(NodeType n : options) {
            if(DEPTH_FACTORS[static_cast<size_t>(n)] < min_depth_factor) {
                min_depth_factor = DEPTH_FACTORS[static_cast<size_t>(n)];
            }
        }

        std::vector<NodeType> new_options;
        for(NodeType n : options) {
            if(DEPTH_FACTORS[static_cast<size_t>(n)] == min_depth_factor)
                new_options.push_back(n);
        }

        options = new_options;
    }

    return options[this->genRandomInt(0, options.size())];
}