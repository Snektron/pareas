#ifndef _PAREAS_LLPGEN_PARSE_TABLE_HPP
#define _PAREAS_LLPGEN_PARSE_TABLE_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/admissible_pair.hpp"

#include <vector>
#include <unordered_map>

struct LLPTable {
    struct Entry {
        std::vector<Symbol> initial_stack;
        std::vector<Symbol> final_stack;
        const Production* prod;
    };

    std::unordered_map<AdmissiblePair, Entry> table;
};

#endif
