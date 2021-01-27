#ifndef _PAREAS_LLPGEN_ADMISSIBLE_PAIR_HPP
#define _PAREAS_LLPGEN_ADMISSIBLE_PAIR_HPP

#include "llpgen/grammar.hpp"

#include <functional>
#include <cstddef>

struct AdmissiblePair {
    Terminal x;
    Terminal y;
};

template <>
struct std::hash<AdmissiblePair> {
    size_t operator()(const AdmissiblePair& ap) const;
};

bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs);

#endif
