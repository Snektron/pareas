#ifndef _PAREAS_LLPGEN_LLP_PSLS_TABLE_HPP
#define _PAREAS_LLPGEN_LLP_PSLS_TABLE_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/llp/admissible_pair.hpp"

#include <unordered_map>
#include <vector>
#include <span>
#include <iosfwd>
#include <stdexcept>

namespace llp {
    struct PSLSConflictError : public InvalidGrammarError {
        PSLSConflictError(): InvalidGrammarError("PSLS conflict: Grammar is not LLP(1, 1)") {}
    };

    struct PSLSTable {
        struct Entry {
            std::vector<Symbol> gamma;
            const Production* prod;
        };

        std::unordered_map<AdmissiblePair, Entry> table;

        void insert(const AdmissiblePair& ap, std::span<const Symbol> symbols);
        void dump_csv(std::ostream& os) const;
    };
}

#endif
