#include "pareas/llpgen/llp/admissible_pair.hpp"
#include "pareas/common/hash_util.hpp"

namespace pareas::llp {
    bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
}

size_t std::hash<pareas::llp::AdmissiblePair>::operator()(const pareas::llp::AdmissiblePair& ap) const {
    auto hasher = std::hash<pareas::Terminal>{};
    return pareas::hash_combine(hasher(ap.x), hasher(ap.y));
}