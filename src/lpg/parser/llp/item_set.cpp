#include "pareas/lpg/parser/llp/item_set.hpp"
#include "pareas/lpg/hash_util.hpp"

#include <fmt/ostream.h>

namespace pareas::parser::llp {
    std::unordered_set<Symbol, Symbol::Hash> ItemSet::syms_before_dots() const {
        auto syms = std::unordered_set<Symbol, Symbol::Hash>();
        for (const auto& item : this->items) {
            if (!item.is_dot_at_begin())
                syms.insert(item.sym_before_dot());
        }

        return syms;
    }

    std::unordered_set<Symbol, Symbol::Hash> ItemSet::syms_after_dots() const {
        auto syms = std::unordered_set<Symbol, Symbol::Hash>();
        for (const auto& item : this->items) {
            if (!item.is_dot_at_end())
                syms.insert(item.sym_after_dot());
        }

        return syms;
    }

    void ItemSet::dump(std::ostream& os) const {
        fmt::print(os, "{{ ");
        bool first = true;
        for (const auto& item : this->items) {
            if (first)
                first = false;
            else
                fmt::print(os, "\n  ");
            fmt::print(os, "{}", item);
        }
        fmt::print(os, " }}\n");
    }

    bool operator==(const ItemSet& lhs, const ItemSet& rhs) {
        return lhs.items == rhs.items;
    }

    size_t ItemSet::Hash::operator()(const ItemSet& item_set) const {
        // Need to make sure that the order doesn't matter for this hash.
        return hash_order_independent_range(item_set.items.begin(), item_set.items.end(), Item::Hash{});
    }
}
