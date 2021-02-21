#ifndef _PAREAS_LLPGEN_LLP_PARSING_TABLE_HPP
#define _PAREAS_LLPGEN_LLP_PARSING_TABLE_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/llp/admissible_pair.hpp"

#include <vector>
#include <unordered_map>
#include <iosfwd>

namespace llp {
    struct ParsingTable {
        struct Entry {
            std::vector<Symbol> initial_stack;
            std::vector<Symbol> final_stack;
            std::vector<const Production*> productions;
        };

        std::unordered_map<AdmissiblePair, Entry> table;

        void dump_csv(std::ostream& os);
    };
}

#endif
