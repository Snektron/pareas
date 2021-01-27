#ifndef _PAREAS_LLPGEN_ITEM_HPP
#define _PAREAS_LLPGEN_ITEM_HPP

#include "pareas/llpgen/grammar.hpp"

#include <vector>
#include <iosfwd>
#include <span>
#include <cstddef>

struct Item {
    const Production* prod;
    size_t dot;
    Terminal lookback;
    Terminal lookahead;
    std::vector<Symbol> gamma;

    static Item initial(const Grammar& g);

    bool is_dot_at_end() const;
    bool is_dot_at_begin() const;

    Symbol sym_at_dot() const;
    Symbol sym_before_dot() const;

    std::span<const Symbol> syms_before_dot() const;
    std::span<const Symbol> syms_after_dot() const;
};

bool operator==(const Item& lhs, const Item& rhs);

std::ostream& operator<<(std::ostream& os, const Item& item);

template <>
struct std::hash<Item> {
    size_t operator()(const Item& item) const;
};

#endif
