#ifndef _PAREAS_LPG_PARSER_LLP_ADMISSIBLE_PAIR_HPP
#define _PAREAS_LPG_PARSER_LLP_ADMISSIBLE_PAIR_HPP

#include "pareas/lpg/parser/grammar.hpp"

#include <cstddef>

namespace pareas::parser::llp {
    struct AdmissiblePair {
        Terminal x;
        Terminal y;

        struct Hash {
            size_t operator()(const AdmissiblePair& ap) const;
        };
    };

    bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs);
}

#endif
