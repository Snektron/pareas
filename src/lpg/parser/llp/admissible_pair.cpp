#include "pareas/lpg/parser/llp/admissible_pair.hpp"
#include "pareas/lpg/hash_util.hpp"

namespace pareas::parser::llp {
    bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
}

size_t std::hash<pareas::parser::llp::AdmissiblePair>::operator()(const pareas::parser::llp::AdmissiblePair& ap) const {
    auto hasher = std::hash<pareas::parser::Terminal>{};
    return pareas::hash_combine(hasher(ap.x), hasher(ap.y));
}