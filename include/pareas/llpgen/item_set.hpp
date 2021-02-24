#ifndef _PAREAS_LLPGEN_ITEM_SET_HPP
#define _PAREAS_LLPGEN_ITEM_SET_HPP

#include "pareas/llpgen/hash_util.hpp"

#include <fmt/ostream.h>

#include <unordered_set>
#include <iosfwd>
#include <concepts>

namespace pareas {
    template <typename T>
    concept Item = requires (T a, T b, std::ostream& os) {
        // Hash requirements
        { std::hash<T>{}(a) } -> std::convertible_to<size_t>;
        { a == b } -> std::same_as<bool>;

        // syms_before_dots/syms_after_dots requirements
        { a.sym_after_dot() } -> std::same_as<Symbol>;
        { a.sym_before_dot() } -> std::same_as<Symbol>;
        { a.is_dot_at_begin() } -> std::same_as<bool>;
        { a.is_dot_at_end() } -> std::same_as<bool>;

        // Dump requirements
        { os << a } -> std::same_as<std::ostream&>;
    };

    template <Item T>
    struct ItemSet {
        std::unordered_set<T> items;

        std::unordered_set<Symbol> syms_before_dots() const;
        std::unordered_set<Symbol> syms_after_dots() const;
        void dump(std::ostream& os) const;
    };

    template <Item T>
    bool operator==(const ItemSet<T>& lhs, const ItemSet<T>& rhs);

    template <Item T>
    std::unordered_set<Symbol> ItemSet<T>::syms_before_dots() const {
        auto syms = std::unordered_set<Symbol>();
        for (const auto& item : this->items) {
            if (!item.is_dot_at_begin())
                syms.insert(item.sym_before_dot());
        }

        return syms;
    }

    template <Item T>
    std::unordered_set<Symbol> ItemSet<T>::syms_after_dots() const {
        auto syms = std::unordered_set<Symbol>();
        for (const auto& item : this->items) {
            if (!item.is_dot_at_end())
                syms.insert(item.sym_after_dot());
        }

        return syms;
    }

    template <Item T>
    void ItemSet<T>::dump(std::ostream& os) const {
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

    template <Item T>
    bool operator==(const ItemSet<T>& lhs, const ItemSet<T>& rhs) {
        return lhs.items == rhs.items;
    }
}

template <pareas::Item T>
struct std::hash<pareas::ItemSet<T>> {
    size_t operator()(const pareas::ItemSet<T>& item) const;
};

template <pareas::Item T>
size_t std::hash<pareas::ItemSet<T>>::operator()(const pareas::ItemSet<T>& item) const {
    // need to make sure that the order doesn't matter for this hash
    // so just use an XOR on the hashes of the items.
    auto hasher = std::hash<T>{};
    size_t hash = 0;
    for (const auto& item : item.items) {
        hash ^= hasher(item);
    }
    return hash;
}

#endif
