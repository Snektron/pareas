#ifndef _PAREAS_LLPGEN_LLP_ITEM_SET_HPP
#define _PAREAS_LLPGEN_LLP_ITEM_SET_HPP

#include "pareas/llpgen/llp/item.hpp"

#include <unordered_set>
#include <iosfwd>

namespace llp {
    struct ItemSet {
        std::unordered_set<Item> items;

        std::unordered_set<Symbol> syms_before_dots() const;
        void dump(std::ostream& os) const;
    };

    bool operator==(const ItemSet& lhs, const ItemSet& rhs);
}

template <>
struct std::hash<llp::ItemSet> {
    size_t operator()(const llp::ItemSet& config) const;
};

#endif
