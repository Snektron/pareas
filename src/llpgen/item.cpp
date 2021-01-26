#include "llpgen/item.hpp"
#include "llpgen/hash_util.hpp"

#include <algorithm>
#include <stdexcept>

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
        .lookahead = g.right_delim,
        .lookback = Terminal::null(),
        .gamma = {}, // TODO: Find out what gamma is
    };
}

bool operator==(const Item& lhs, const Item& rhs) {
    return lhs.prod == rhs.prod &&
        lhs.dot == rhs.dot &&
        lhs.lookahead == rhs.lookahead &&
        lhs.lookback == rhs.lookback &&
        std::equal(lhs.gamma.begin(), lhs.gamma.end(), rhs.gamma.begin(), rhs.gamma.end());
}

size_t std::hash<Item>::operator()(const Item& item) const {
    size_t hash = std::hash<const Production*>{}(item.prod);
    hash = hash_combine(hash, std::hash<size_t>{}(item.dot));
    hash = hash_combine(hash, std::hash<Terminal>{}(item.lookahead));
    hash = hash_combine(hash, std::hash<Terminal>{}(item.lookback));
    hash = hash_combine(hash, hash_iterator_range(item.gamma.begin(), item.gamma.end(), std::hash<Symbol>{}));
    return hash;
}
