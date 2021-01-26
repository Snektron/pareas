#include "llpgen/configuration.hpp"
#include "llpgen/hash_util.hpp"

bool operator==(const Configuration& lhs, const Configuration& rhs) {
    if (lhs.items.size() != rhs.items.size())
        return false;

    for (const auto& item : lhs.items) {
        if (rhs.items.find(item) == rhs.items.end())
            return false;
    }

    for (const auto& item : rhs.items) {
        if (lhs.items.find(item) == lhs.items.end())
            return false;
    }

    return true;
}

size_t std::hash<Configuration>::operator()(const Configuration& config) const {
    // need to make sure that the order doesn't matter for this hash
    // so just use an XOR on the hashes of the items.
    auto hasher = std::hash<Item>{};
    size_t hash = 0;
    for (const auto& item : config.items) {
        hash ^= hasher(item);
    }
    return hash;
}
