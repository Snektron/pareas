#ifndef _PAREAS_LPG_PARSER_LLP_ITEM_SET_HPP
#define _PAREAS_LPG_PARSER_LLP_ITEM_SET_HPP

#include "pareas/lpg/parser/llp/item.hpp"
#include "pareas/lpg/parser/grammar.hpp"

#include <iosfwd>
#include <unordered_set>
#include <cstddef>

namespace pareas::parser::llp {
    struct ItemSet {
        std::unordered_set<Item, Item::Hash> items;

        std::unordered_set<Symbol, Symbol::Hash> syms_before_dots() const;
        std::unordered_set<Symbol, Symbol::Hash> syms_after_dots() const;
        void dump(std::ostream& os) const;

        struct Hash {
            size_t operator()(const ItemSet& item_set) const;
        };
    };

    bool operator==(const ItemSet& lhs, const ItemSet& rhs);
}

#endif
