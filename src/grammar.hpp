#ifndef _PAREAS_GRAMMAR_H
#define _PAREAS_GRAMMAR_H

#include <unordered_map>
#include <vector>
#include <variant>

template <typename NT, typename T>
struct Grammar {
    using NonTerminal = NT;
    using Terminal = T;
    using Symbol = std::variant<NonTerminal, Terminal>;
    using Production = std::vector<Symbol>;

    NonTerminal start;
    Terminal delim;

    std::unordered_multimap<NonTerminal, Production> productions;
};

#endif
