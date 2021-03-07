#include "pareas/lpg/parser/llp/admissible_pair.hpp"
#include "pareas/lpg/hash_util.hpp"

namespace pareas::parser::llp {
    size_t AdmissiblePair::Hash::operator()(const AdmissiblePair& ap) const {
        auto hasher = Terminal::Hash{};
        return hash_combine(hasher(ap.x), hasher(ap.y));
    }

    bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
}
