#ifndef _PAREAS_LLPGEN_LLP_GENERATOR_HPP
#define _PAREAS_LLPGEN_LLP_GENERATOR_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/error_reporter.hpp"
#include "pareas/llpgen/item_set.hpp"
#include "pareas/llpgen/ll/parsing_table.hpp"
#include "pareas/llpgen/llp/item.hpp"
#include "pareas/llpgen/llp/psls_table.hpp"
#include "pareas/llpgen/llp/parsing_table.hpp"

#include <unordered_set>
#include <iosfwd>

namespace pareas::llp {
    using LLPItemSet = ItemSet<Item>;

    class Generator {
        ErrorReporter* er;
        const Grammar* g;
        const TerminalSetFunctions* tsf;

        std::unordered_set<LLPItemSet> item_sets;

    public:
        Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf);
        PSLSTable build_psls_table();
        ParsingTable build_parsing_table(const ll::ParsingTable& ll, const PSLSTable& psls);
        void dump(std::ostream& os);

    private:
        void compute_item_sets();
        LLPItemSet predecessor(const LLPItemSet& set, const Symbol& sym);
        void closure(LLPItemSet& set);
        std::vector<Symbol> compute_gamma(const Terminal& v, const Symbol& x, std::span<const Symbol> delta);
    };
}

#endif
