#include "llpgen/item.hpp"
#include "llpgen/hash_util.hpp"

#include <algorithm>
#include <stdexcept>
#include <ostream>
#include <cassert>

Item Item::initial(const Grammar& g) {
    // Find the start rule
    // Only one is allowed
    const Production* start = nullptr;
    for (const auto& prod : g.productions) {
        if (prod.lhs != g.start)
            continue;
        if (start)
            throw std::runtime_error("Start rule appears in multiple productions");
        start = &prod;
    }

    // Verify that the starting rule is of the right form
    if (start->rhs.size() == 0 || start->rhs.front() != g.left_delim || start->rhs.back() != g.right_delim)
        throw std::runtime_error("Start rule not in right form");

    return {
        .prod = start,
        .dot = start->rhs.size(),
        .lookback = g.right_delim,
        .lookahead = Terminal::null(),
        .gamma = {}, // TODO: Find out what gamma is
    };
}

bool Item::is_dot_at_end() const {
    return this->dot == this->prod->rhs.size();
}

bool Item::is_dot_at_begin() const {
    return this->dot == 0;
}

Symbol Item::sym_at_dot() const {
    assert(!this->is_dot_at_end());
    return this->prod->rhs[this->dot];
}

Symbol Item::sym_before_dot() const {
    assert(!this->is_dot_at_begin());
    return this->prod->rhs[this->dot - 1];
}

std::span<const Symbol> Item::syms_before_dot() const {
    return std::span(this->prod->rhs).subspan(0, this->dot);
}

std::span<const Symbol> Item::syms_after_dot() const {
    return std::span(this->prod->rhs).subspan(this->dot + 1);
}

bool operator==(const Item& lhs, const Item& rhs) {
    return lhs.prod == rhs.prod &&
        lhs.dot == rhs.dot &&
        lhs.lookahead == rhs.lookahead &&
        lhs.lookback == rhs.lookback &&
        std::equal(lhs.gamma.begin(), lhs.gamma.end(), rhs.gamma.begin(), rhs.gamma.end());
}

std::ostream& operator<<(std::ostream& os, const Item& item) {
    os << "[" << Symbol(item.prod->lhs) << " ->";
    for (size_t i = 0; i < item.prod->rhs.size(); ++i) {
        if (item.dot == i)
            os << " •";
        os << " " << item.prod->rhs[i];
    }

    if (item.is_dot_at_end())
        os << " •";

    os << ", " << item.lookback << ", " << item.lookahead << ",";

    if (item.gamma.empty()) {
        os << " ε";
    } else {
        for (const auto& sym : item.gamma) {
            os << " " << sym;
        }
    }

    return os << "]";
}

size_t std::hash<Item>::operator()(const Item& item) const {
    size_t hash = std::hash<const Production*>{}(item.prod);
    hash = hash_combine(hash, std::hash<size_t>{}(item.dot));
    hash = hash_combine(hash, std::hash<Terminal>{}(item.lookahead));
    hash = hash_combine(hash, std::hash<Terminal>{}(item.lookback));
    hash = hash_combine(hash, hash_iterator_range(item.gamma.begin(), item.gamma.end(), std::hash<Symbol>{}));
    return hash;
}
