#include "pareas/llpgen/llp/llp_item.hpp"
#include "pareas/llpgen/hash_util.hpp"

#include <algorithm>
#include <stdexcept>
#include <ostream>
#include <cassert>

namespace llp {
    bool LLPItem::is_dot_at_end() const {
        return this->dot == this->prod->rhs.size();
    }

    bool LLPItem::is_dot_at_begin() const {
        return this->dot == 0;
    }

    Symbol LLPItem::sym_after_dot() const {
        assert(!this->is_dot_at_end());
        return this->prod->rhs[this->dot];
    }

    Symbol LLPItem::sym_before_dot() const {
        assert(!this->is_dot_at_begin());
        return this->prod->rhs[this->dot - 1];
    }

    std::span<const Symbol> LLPItem::syms_before_dot() const {
        return std::span(this->prod->rhs).subspan(0, this->dot);
    }

    std::span<const Symbol> LLPItem::syms_after_dot() const {
        return std::span(this->prod->rhs).subspan(this->dot + 1);
    }

    bool operator==(const LLPItem& lhs, const LLPItem& rhs) {
        return lhs.prod == rhs.prod &&
            lhs.dot == rhs.dot &&
            lhs.lookahead == rhs.lookahead &&
            lhs.lookback == rhs.lookback &&
            std::equal(lhs.gamma.begin(), lhs.gamma.end(), rhs.gamma.begin(), rhs.gamma.end());
    }

    std::ostream& operator<<(std::ostream& os, const LLPItem& item) {
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
}

size_t std::hash<llp::LLPItem>::operator()(const llp::LLPItem& item) const {
    size_t hash = std::hash<const Production*>{}(item.prod);
    hash = hash_combine(hash, std::hash<size_t>{}(item.dot));
    hash = hash_combine(hash, std::hash<Terminal>{}(item.lookahead));
    hash = hash_combine(hash, std::hash<Terminal>{}(item.lookback));
    hash = hash_combine(hash, hash_iterator_range(item.gamma.begin(), item.gamma.end(), std::hash<Symbol>{}));
    return hash;
}
