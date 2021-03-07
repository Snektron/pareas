#ifndef _PAREAS_LPG_PARSER_LLP_ITEM_HPP
#define _PAREAS_LPG_PARSER_LLP_ITEM_HPP

#include "pareas/lpg/parser/grammar.hpp"

#include <vector>
#include <iosfwd>
#include <span>
#include <cstddef>

namespace pareas::parser::llp {
    struct Item {
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

        struct Hash {
            size_t operator()(const Item& item) const;
        };
    };

    bool operator==(const Item& lhs, const Item& rhs);

    std::ostream& operator<<(std::ostream& os, const Item& item);
}

#endif
