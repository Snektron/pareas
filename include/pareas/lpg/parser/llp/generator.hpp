#ifndef _PAREAS_LPG_PARSER_LLP_GENERATOR_HPP
#define _PAREAS_LPG_PARSER_LLP_GENERATOR_HPP

#include "pareas/lpg/error_reporter.hpp"
#include "pareas/lpg/parser/grammar.hpp"
#include "pareas/lpg/parser/terminal_set_functions.hpp"
#include "pareas/lpg/parser/ll/parsing_table.hpp"
#include "pareas/lpg/parser/llp/item.hpp"
#include "pareas/lpg/parser/llp/item_set.hpp"
#include "pareas/lpg/parser/llp/psls_table.hpp"
#include "pareas/lpg/parser/llp/parsing_table.hpp"

#include <unordered_set>
#include <iosfwd>

namespace pareas::parser::llp {
    class Generator {
        ErrorReporter* er;
        const Grammar* g;
        const TerminalSetFunctions* tsf;

        std::unordered_set<ItemSet, ItemSet::Hash> item_sets;

    public:
        Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf);
        PSLSTable build_psls_table();
        ParsingTable build_parsing_table(const ll::ParsingTable& ll, const PSLSTable& psls);
        void dump(std::ostream& os);

    private:
        void compute_item_sets();
        ItemSet predecessor(const ItemSet& set, const Symbol& sym);
        void closure(ItemSet& set);
        std::vector<Symbol> compute_gamma(const Terminal& v, const Symbol& x, std::span<const Symbol> delta);
    };
}

#endif
