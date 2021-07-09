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
        uint32_t min_int_const, uint32_t max_int_const, float min_flt_const, float max_flt_const,
        size_t max_id_len, size_t max_stat_list_len, size_t max_func_list_len, size_t max_func_arg_list_len) : 
    rng(seed), target_width(target_width), target_height(target_height),
    min_int_const(min_int_const), max_int_const(max_int_const), min_flt_const(min_flt_const), max_flt_const(max_flt_const),
    max_id_length(max_id_len), max_stat_list_len(max_stat_list_len), max_func_list_len(max_func_list_len),
    max_func_arg_list_len(max_func_arg_list_len) {}

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

DataType ASTGenerator::pickRetType(const std::vector<DataType>& valid_options) {
    return valid_options[this->genRandomInt(0, valid_options.size())];
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

size_t ASTGenerator::randomStatementListLen(NodeType node_type) {
    size_t len_max;
    switch(node_type) {
        case NodeType::STATEMENT_LIST:
            len_max = this->max_stat_list_len;
            break;
        case NodeType::FUNC_ARG_LIST:
            len_max = this->max_func_arg_list_len;
            break;
        case NodeType::FUNC_DECL_LIST:
            len_max = this->max_func_list_len;
            break;
        default:
            len_max = 0; //Invalid
            break;
    }

    std::binomial_distribution<size_t> distr(len_max, 0.5);
    size_t len = distr(this->rng);

    if(this->current_depth < this->current_widths.size()) {
        size_t next_layer_width = this->current_widths[this->current_depth];
        if(next_layer_width + len >= this->target_width) {
            if(next_layer_width >= this->target_width)
                len = 0;
            else
                len = this->target_width - next_layer_width;
        }
    }

    if(node_type != NodeType::FUNC_ARG_LIST)
        len = len > 0 ? len : 1;

    return len;
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


std::string ASTGenerator::findSymbol(DataType datatype) {
    std::vector<std::string> candidates;
    for(size_t i = scopes.size(); i > 0; --i) {
        auto& layer = scopes[i-1];

        for(auto& it : layer) {
            if(it.second == datatype) {
                candidates.push_back(it.first);
            }
        }
    }

    if(candidates.size() == 0)
        return "";
    else
        return candidates[this->genRandomInt(0, candidates.size())];
}

std::pair<std::string, std::vector<DataType>> ASTGenerator::findFunction(DataType retval) {
    std::vector<std::string> candidates;

    for(auto& it : this->functab) {
        if(it.second.first == retval) {
            candidates.push_back(it.first);
        }
    }

    if(candidates.size() == 0)
        return std::pair<std::string, std::vector<DataType>>("", {});
    else {
        std::string choice = candidates[this->genRandomInt(0, candidates.size())];
        std::vector<DataType> args = this->functab[choice].second;
        return std::pair<std::string, std::vector<DataType>>(choice, args);
    }
}

std::string ASTGenerator::makeSymbol(DataType datatype) {
    std::string name;
    do {
        name = this->randomId();
    }
    while(this->changed_scope.count(name) > 0);
    this->changed_scope[name] = datatype;

    return name;
}

std::string ASTGenerator::makeFunction(DataType rettype, const std::vector<DataType>& arg_types) {
    std::string name;
    do {
        name = this->randomId();
    }
    while(this->functab.count(name) > 0);

    this->functab[name] = {rettype, arg_types};

    return name;
}

std::string ASTGenerator::randomId() {
    auto random_id_char = [&](size_t lim) {
        size_t x = this->genRandomInt(0, lim);
        if(x < 26)
            return 'A' + x;
        if(x < 2*26)
            return 'a' + (x - 26);
        return '0' + (x - 2*26);
    };

    std::string result;
    result.push_back(random_id_char(2*26));

    std::binomial_distribution<size_t> distr(this->max_id_length, 0.5);
    size_t len = distr(this->rng);

    for(size_t i = 0; i <= len; ++i) {
        result.push_back(random_id_char(2*26+10));
    }
    return result;
}