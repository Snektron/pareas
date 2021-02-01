#ifndef _PAREAS_LLPGEN_LLP_GENERATOR_HPP
#define _PAREAS_LLPGEN_LLP_GENERATOR_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/error_reporter.hpp"
#include "pareas/llpgen/ll.hpp"
#include "pareas/llpgen/llp/item_set.hpp"
#include "pareas/llpgen/llp/psls_table.hpp"
#include "pareas/llpgen/llp/llp_table.hpp"

#include <unordered_set>
#include <iosfwd>

namespace llp {
    class Generator {
        ErrorReporter* er;
        const Grammar* g;
        const TerminalSetFunctions* tsf;

        std::unordered_set<ItemSet> item_sets;

    public:
        Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf);
        PSLSTable build_psls_table();
        LLPTable build_llp_table(const ll::LLTable& ll, const PSLSTable& psls);
        void dump(std::ostream& os);

    private:
        void compute_item_sets();
        ItemSet predecessor(const ItemSet& set, const Symbol& sym);
        void closure(ItemSet& set);
        std::vector<Symbol> compute_gamma(const Terminal& v, const Symbol& x, std::span<const Symbol> delta);
    };
}

#endif
