#include "pareas/lpg/parser/llp/item.hpp"
#include "pareas/lpg/hash_util.hpp"

#include <fmt/ostream.h>

#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace pareas::parser::llp {
    bool Item::is_dot_at_end() const {
        return this->dot == this->prod->rhs.size();
    }

    bool Item::is_dot_at_begin() const {
        return this->dot == 0;
    }

    Symbol Item::sym_after_dot() const {
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
        fmt::print(os, "[{} ->", Symbol(item.prod->lhs));

        for (size_t i = 0; i < item.prod->rhs.size(); ++i) {
            if (item.dot == i)
                fmt::print(os, " •");
            fmt::print(os, " {}", item.prod->rhs[i]);
        }

        if (item.is_dot_at_end())
            fmt::print(os, " •");

        fmt::print(os, ", {}, {},", item.lookback, item.lookahead);

        if (item.gamma.empty()) {
            fmt::print(os, " ε");
        } else {
            for (const auto& sym : item.gamma) {
                fmt::print(os, " {}", sym);
            }
        }

        fmt::print(os, "]");
        return os;
    }
}

size_t std::hash<pareas::parser::llp::Item>::operator()(const pareas::parser::llp::Item& item) const {
    size_t hash = std::hash<const pareas::parser::Production*>{}(item.prod);
    hash = pareas::hash_combine(hash, std::hash<size_t>{}(item.dot));
    hash = pareas::hash_combine(hash, std::hash<pareas::parser::Terminal>{}(item.lookahead));
    hash = pareas::hash_combine(hash, std::hash<pareas::parser::Terminal>{}(item.lookback));
    hash = pareas::hash_combine(hash, pareas::hash_iterator_range(item.gamma.begin(), item.gamma.end(), std::hash<pareas::parser::Symbol>{}));
    return hash;
}
