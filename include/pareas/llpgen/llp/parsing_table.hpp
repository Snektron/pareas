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

        // TODO: This start rule can be removed by integrating it with other
        // rules starting with the left delimiter.
        // This also requires some fixup in the renderer, as the RBR should originally
        // be ommitted for the stack change associated with the start entry. This can be
        // solved either by also omitting it from these new combined rule, or also adding a
        // stack pop of the start rule to any right delimiter pair
        Entry start;
        std::unordered_map<AdmissiblePair, Entry> table;

        void dump_csv(std::ostream& os);
    };
}

#endif
