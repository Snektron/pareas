#ifndef _PAREAS_LLPGEN_LR_LR_ITEM_HPP
#define _PAREAS_LLPGEN_LR_LR_ITEM_HPP

#include "pareas/llpgen/grammar.hpp"

namespace lr {
    struct LRItem {
        const Production* prod;
        size_t dot;
        Terminal lookahead;

        bool is_dot_at_end() const;
        bool is_dot_at_begin() const;

        Symbol sym_after_dot() const;
        Symbol sym_before_dot() const;
    };

    bool operator==(const LRItem& lhs, const LRItem& rhs);

    std::ostream& operator<<(std::ostream& os, const LRItem& item);
}

template <>
struct std::hash<lr::LRItem> {
    size_t operator()(const lr::LRItem& item) const;
};

#endif
