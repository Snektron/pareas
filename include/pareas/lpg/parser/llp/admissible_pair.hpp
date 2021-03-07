#ifndef _PAREAS_LPG_PARSER_LLP_ADMISSIBLE_PAIR_HPP
#define _PAREAS_LPG_PARSER_LLP_ADMISSIBLE_PAIR_HPP

#include "pareas/lpg/parser/grammar.hpp"

#include <functional>
#include <cstddef>

namespace pareas::parser::llp {
    struct AdmissiblePair {
        Terminal x;
        Terminal y;
    };

    bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs);
}

template <>
struct std::hash<pareas::parser::llp::AdmissiblePair> {
    size_t operator()(const pareas::parser::llp::AdmissiblePair& ap) const;
};


#endif
