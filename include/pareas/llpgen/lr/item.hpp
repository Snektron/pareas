#ifndef _PAREAS_LLPGEN_LR_ITEM_HPP
#define _PAREAS_LLPGEN_LR_ITEM_HPP

#include "pareas/llpgen/grammar.hpp"

#include <span>
#include <cstddef>

namespace lr {
    struct Item {
        const Production* prod;
        size_t dot;
        Terminal lookahead;

        bool is_dot_at_end() const;
        bool is_dot_at_begin() const;

        Symbol sym_after_dot() const;
        Symbol sym_before_dot() const;

        std::span<const Symbol> syms_after_dot() const;
    };

    bool operator==(const Item& lhs, const Item& rhs);

    std::ostream& operator<<(std::ostream& os, const Item& item);
}

template <>
struct std::hash<lr::Item> {
    size_t operator()(const lr::Item& item) const;
};

#endif
