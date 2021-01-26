#ifndef _PAREAS_LLPGEN_GENERATOR_HPP
#define _PAREAS_LLPGEN_GENERATOR_HPP

#include "llpgen/grammar.hpp"

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

public:
    LLPGenerator(const Grammar* g);
    void dump(std::ostream& os);

private:
    std::unordered_map<NonTerminal, TerminalSet> compute_base_first_or_last_set(bool first);
    TerminalSet compute_first_or_last_set(std::span<const Symbol> symbols, bool first);
    std::unordered_map<NonTerminal, TerminalSet> compute_follow_or_before_sets(bool follow);
};

#endif
