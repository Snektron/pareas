#ifndef _PAREAS_LLPGEN_PSLS_HPP
#define _PAREAS_LLPGEN_PSLS_HPP

#include "llpgen/grammar.hpp"

#include <unordered_map>
#include <vector>
#include <span>
#include <functional>
#include <iosfwd>
#include <cstddef>

struct AdmissiblePair {
    Terminal x;
    Terminal y;
};

template <>
struct std::hash<AdmissiblePair> {
    size_t operator()(const AdmissiblePair& terms) const;
};

bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs);

struct PSLSTable {
    std::unordered_map<AdmissiblePair, std::vector<Symbol>> table;

    void insert(const AdmissiblePair& terms, std::span<const Symbol> symbols);
    void dump_csv(std::ostream& os) const;
};


#endif
