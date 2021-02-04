#ifndef _PAREAS_LLPGEN_LLP_LLP_GENERATOR_HPP
#define _PAREAS_LLPGEN_LLP_LLP_GENERATOR_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/error_reporter.hpp"
#include "pareas/llpgen/ll.hpp"
#include "pareas/llpgen/item_set.hpp"
#include "pareas/llpgen/llp/llp_item.hpp"
#include "pareas/llpgen/llp/psls_table.hpp"
#include "pareas/llpgen/llp/llp_table.hpp"

#include <unordered_set>
#include <iosfwd>

namespace llp {
    using LLPItemSet = ItemSet<LLPItem>;

    class LLPGenerator {
        ErrorReporter* er;
        const Grammar* g;
        const TerminalSetFunctions* tsf;

        std::unordered_set<LLPItemSet> item_sets;

    public:
        LLPGenerator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf);
        PSLSTable build_psls_table();
        LLPTable build_llp_table(const ll::LLTable& ll, const PSLSTable& psls);
        void dump(std::ostream& os);

    private:
        void compute_item_sets();
        LLPItemSet predecessor(const LLPItemSet& set, const Symbol& sym);
        void closure(LLPItemSet& set);
        std::vector<Symbol> compute_gamma(const Terminal& v, const Symbol& x, std::span<const Symbol> delta);
    };
}

#endif
