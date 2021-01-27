#ifndef _PAREAS_LLPGEN_ITEM_SET_HPP
#define _PAREAS_LLPGEN_ITEM_SET_HPP

#include "pareas/llpgen/item.hpp"

#include <unordered_set>
#include <iosfwd>

struct ItemSet {
    std::unordered_set<Item> items;

    std::unordered_set<Symbol> syms_before_dots() const;
    void dump(std::ostream& os) const;
};

bool operator==(const ItemSet& lhs, const ItemSet& rhs);

template <>
struct std::hash<ItemSet> {
    size_t operator()(const ItemSet& config) const;
};

#endif
