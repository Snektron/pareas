#ifndef _PAREAS_LLPGEN_LR_LR_GENERATOR_HPP
#define _PAREAS_LLPGEN_LR_LR_GENERATOR_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/error_reporter.hpp"
#include "pareas/llpgen/item_set.hpp"
#include "pareas/llpgen/lr/lr_item.hpp"
#include "pareas/llpgen/lr/lr_table.hpp"

#include <unordered_map>
#include <iosfwd>

namespace lr {
    using LRItemSet = ItemSet<LRItem>;

    class LRGenerator {
        ErrorReporter* er;
        const Grammar* g;
        const TerminalSetFunctions* tsf;

        std::unordered_map<LRItemSet, size_t> item_sets;

    public:
        LRGenerator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf);
        LRTable build_lr_table();
        void dump(std::ostream& os) const;

    private:
        LRItemSet successor(const LRItemSet& set, const Symbol& sym);
        void closure(LRItemSet& set);
    };
}

#endif
