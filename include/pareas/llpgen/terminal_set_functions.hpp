#ifndef _PAREAS_LLPGEN_TERMINAL_SET_FUNCTIONS_HPP
#define _PAREAS_LLPGEN_TERMINAL_SET_FUNCTIONS_HPP

#include "pareas/llpgen/grammar.hpp"

#include <unordered_map>
#include <unordered_set>
#include <span>
#include <iosfwd>

using TerminalSet = std::unordered_set<Terminal>;
using TerminalSetMap = std::unordered_map<NonTerminal, TerminalSet>;

struct TerminalSetFunctions {
    TerminalSetMap base_first_sets;
    TerminalSetMap base_last_sets;

    TerminalSetMap follow_sets;
    TerminalSetMap before_sets;

    TerminalSetFunctions(const Grammar& g);

    const TerminalSet& first(const NonTerminal& nt) const;
    const TerminalSet& last(const NonTerminal& nt) const;

    const TerminalSet& follow(const NonTerminal& nt) const;
    const TerminalSet& before(const NonTerminal& nt) const;

    TerminalSet compute_first(std::span<const Symbol> symbols) const;
    TerminalSet compute_last(std::span<const Symbol> symbols) const;

    void dump(std::ostream& os);
private:
    TerminalSetMap compute_base_first_or_last_set(const Grammar& g, bool first) const;
    TerminalSetMap compute_follow_or_before_sets(const Grammar& g, bool follow) const;

    TerminalSet compute_first_or_last_set(std::span<const Symbol> symbols, bool first) const;
};

#endif
