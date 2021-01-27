#ifndef _PAREAS_LLPGEN_LL_TABLE_HPP
#define _PAREAS_LLPGEN_LL_TABLE_HPP

#include "pareas/llpgen/grammar.hpp"

#include <unordered_map>
#include <functional>
#include <iosfwd>
#include <stdexcept>
#include <cstddef>

struct LLTableKey {
    NonTerminal nt;
    Terminal t;
};

bool operator==(const LLTableKey& lhs, const LLTableKey& rhs);

template <>
struct std::hash<LLTableKey> {
    size_t operator()(const LLTableKey& key) const;
};

struct LLConflictError: public InvalidGrammarError {
    LLTableKey a;
    LLTableKey b;

    LLConflictError(const LLTableKey& a, const LLTableKey& b);
};

struct LLTable {
    std::unordered_map<LLTableKey, const Production*> table;

    void insert(const LLTableKey& key, const Production* prod);
    std::vector<const Production*> partial_parse(const Terminal& y, std::vector<Symbol>& stack) const;
    void dump_csv(std::ostream& os) const;
};

#endif
