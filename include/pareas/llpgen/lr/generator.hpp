#ifndef _PAREAS_LLPGEN_LR_GENERATOR_HPP
#define _PAREAS_LLPGEN_LR_GENERATOR_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/item_set.hpp"
#include "pareas/llpgen/lr/item.hpp"
#include "pareas/llpgen/lr/parsing_table.hpp"

#include <unordered_map>
#include <iosfwd>

namespace lr {
    using LRItemSet = ItemSet<Item>;

    class Generator {
        const Grammar* g;
        const TerminalSetFunctions* tsf;

        std::unordered_map<LRItemSet, size_t> item_sets;

    public:
        Generator(const Grammar* g, const TerminalSetFunctions* tsf);
        ParsingTable build_parsing_table();
        void dump(std::ostream& os) const;

    private:
        LRItemSet successor(const LRItemSet& set, const Symbol& sym);
        void closure(LRItemSet& set);
    };
}

#endif
