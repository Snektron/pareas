#ifndef _PAREAS_LLPGEN_ITEM_HPP
#define _PAREAS_LLPGEN_ITEM_HPP

#include "llpgen/grammar.hpp"

#include <vector>
#include <cstddef>

struct Item {
    const Production* prod;
    size_t dot;
    Terminal lookahead;
    Terminal lookback;
    std::vector<Symbol> gamma;

    static Item initial(const Grammar& g);
};

bool operator==(const Item& lhs, const Item& rhs);

template <>
struct std::hash<Item> {
    size_t operator()(const Item& item) const;
};

#endif
