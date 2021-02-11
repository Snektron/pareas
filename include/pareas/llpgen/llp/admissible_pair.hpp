#ifndef _PAREAS_LLPGEN_LLP_ADMISSIBLE_PAIR_HPP
#define _PAREAS_LLPGEN_LLP_ADMISSIBLE_PAIR_HPP

#include "pareas/llpgen/grammar.hpp"

#include <functional>
#include <cstddef>

namespace llp {
    struct AdmissiblePair {
        Terminal x;
        Terminal y;
    };

    bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs);
}

template <>
struct std::hash<llp::AdmissiblePair> {
    size_t operator()(const llp::AdmissiblePair& ap) const;
};


#endif
