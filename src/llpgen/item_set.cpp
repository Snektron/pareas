#include "pareas/llpgen/llp/item_set.hpp"
#include "pareas/llpgen/hash_util.hpp"

#include <ostream>

namespace llp {
    std::unordered_set<Symbol> ItemSet::syms_before_dots() const {
        auto syms = std::unordered_set<Symbol>();
        for (const auto& item : this->items) {
            if (!item.is_dot_at_begin())
                syms.insert(item.sym_before_dot());
        }

        return syms;
    }

    void ItemSet::dump(std::ostream& os) const {
        os << "{ ";
        bool first = true;
        for (const auto& item : this->items) {
            if (first) {
                first = false;
            } else {
                os << std::endl << "  ";
            }
            os << item;
        }
        os << " }" << std::endl;
    }

    bool operator==(const ItemSet& lhs, const ItemSet& rhs) {
        return lhs.items == rhs.items;
    }
}

size_t std::hash<llp::ItemSet>::operator()(const llp::ItemSet& config) const {
    // need to make sure that the order doesn't matter for this hash
    // so just use an XOR on the hashes of the items.
    auto hasher = std::hash<llp::Item>{};
    size_t hash = 0;
    for (const auto& item : config.items) {
        hash ^= hasher(item);
    }
    return hash;
}
