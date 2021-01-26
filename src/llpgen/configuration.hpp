#ifndef _PAREAS_LLPGEN_CONFIGURATION_HPP
#define _PAREAS_LLPGEN_CONFIGURATION_HPP

#include "llpgen/item.hpp"
#include <unordered_set>

struct Configuration {
    std::unordered_set<Item> items;
};

bool operator==(const Configuration& lhs, const Configuration& rhs);

template <>
struct std::hash<Configuration> {
    size_t operator()(const Configuration& config) const;
};

#endif
