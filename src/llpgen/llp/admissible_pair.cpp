#include "pareas/llpgen/llp/admissible_pair.hpp"
#include "pareas/llpgen/hash_util.hpp"

namespace llp {
    bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
}

size_t std::hash<llp::AdmissiblePair>::operator()(const llp::AdmissiblePair& ap) const {
    auto hasher = std::hash<Terminal>{};
    return hash_combine(hasher(ap.x), hasher(ap.y));
}