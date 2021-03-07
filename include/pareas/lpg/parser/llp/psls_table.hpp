#ifndef _PAREAS_LPG_PARSER_LLP_PSLS_TABLE_HPP
#define _PAREAS_LPG_PARSER_LLP_PSLS_TABLE_HPP

#include "pareas/lpg/parser/llp/admissible_pair.hpp"
#include "pareas/lpg/parser/grammar.hpp"

#include <unordered_map>
#include <vector>
#include <span>
#include <iosfwd>
#include <stdexcept>

namespace pareas::llp {
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
