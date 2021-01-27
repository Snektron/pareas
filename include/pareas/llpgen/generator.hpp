#ifndef _PAREAS_LLPGEN_GENERATOR_HPP
#define _PAREAS_LLPGEN_GENERATOR_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/item_set.hpp"
#include "pareas/llpgen/psls_table.hpp"

#include <unordered_set>
#include <unordered_map>
#include <span>
#include <iosfwd>

using TerminalSet = std::unordered_set<Terminal>;

class LLPGenerator {
    const Grammar* g;

    std::unordered_map<NonTerminal, TerminalSet> base_first_sets;
    std::unordered_map<NonTerminal, TerminalSet> base_last_sets;

    std::unordered_map<NonTerminal, TerminalSet> follow_sets;
    std::unordered_map<NonTerminal, TerminalSet> before_sets;

    std::unordered_set<ItemSet> item_sets;

public:
    explicit LLPGenerator(const Grammar* g);
    void dump(std::ostream& os);
    PSLSTable build_psls_table();

private:
    std::unordered_set<ItemSet> compute_item_sets();
    std::unordered_map<NonTerminal, TerminalSet> compute_base_first_or_last_set(bool first);
    TerminalSet compute_first_or_last_set(std::span<const Symbol> symbols, bool first);
    std::unordered_map<NonTerminal, TerminalSet> compute_follow_or_before_sets(bool follow);
    ItemSet predecessor(const ItemSet& set, const Symbol& sym);
    void closure(ItemSet& set);
    std::vector<Symbol> compute_gamma(const Terminal& v, const Symbol& x, std::span<const Symbol> delta);
};

#endif
