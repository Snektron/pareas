#include "llpgen/admissible_pair.hpp"
#include "llpgen/hash_util.hpp"

bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

size_t std::hash<AdmissiblePair>::operator()(const AdmissiblePair& ap) const {
    auto hasher = std::hash<Terminal>{};
    return hash_combine(hasher(ap.x), hasher(ap.y));
}