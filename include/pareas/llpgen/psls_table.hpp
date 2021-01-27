#ifndef _PAREAS_LLPGEN_PSLS_HPP
#define _PAREAS_LLPGEN_PSLS_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/admissible_pair.hpp"

#include <unordered_map>
#include <vector>
#include <span>
#include <iosfwd>
#include <stdexcept>

struct PSLSConflictError : public InvalidGrammarError {
    AdmissiblePair ap;
    std::vector<Symbol> a;
    std::vector<Symbol> b;

    PSLSConflictError(const AdmissiblePair& ap, std::span<const Symbol> a, std::span<const Symbol> b);
};

struct PSLSTable {
    std::unordered_map<AdmissiblePair, std::vector<Symbol>> table;

    void insert(const AdmissiblePair& ap, std::span<const Symbol> symbols);
    void dump_csv(std::ostream& os) const;
};


#endif
