#ifndef _PAREAS_LLPGEN_LLP_LLP_ITEM_HPP
#define _PAREAS_LLPGEN_LLP_LLP_ITEM_HPP

#include "pareas/llpgen/grammar.hpp"

#include <vector>
#include <iosfwd>
#include <span>
#include <cstddef>

namespace llp {
    struct LLPItem {
        const Production* prod;
        size_t dot;
        Terminal lookback;
        Terminal lookahead;
        std::vector<Symbol> gamma;

        bool is_dot_at_end() const;
        bool is_dot_at_begin() const;

        Symbol sym_after_dot() const;
        Symbol sym_before_dot() const;

        std::span<const Symbol> syms_before_dot() const;
        std::span<const Symbol> syms_after_dot() const;
    };

    bool operator==(const LLPItem& lhs, const LLPItem& rhs);

    std::ostream& operator<<(std::ostream& os, const LLPItem& item);
}

template <>
struct std::hash<llp::LLPItem> {
    size_t operator()(const llp::LLPItem& item) const;
};

#endif
