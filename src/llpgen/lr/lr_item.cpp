#include "pareas/llpgen/lr/lr_item.hpp"
#include "pareas/llpgen/hash_util.hpp"

namespace lr {
    bool LRItem::is_dot_at_end() const {
        return this->dot == this->prod->rhs.size();
    }

    bool LRItem::is_dot_at_begin() const {
        return this->dot == 0;
    }

    Symbol LRItem::sym_after_dot() const {
        assert(!this->is_dot_at_end());
        return this->prod->rhs[this->dot];
    }

    Symbol LRItem::sym_before_dot() const {
        assert(!this->is_dot_at_begin());
        return this->prod->rhs[this->dot - 1];
    }

    std::span<const Symbol> LRItem::syms_after_dot() const {
        return std::span(this->prod->rhs).subspan(this->dot);
    }

    bool operator==(const LRItem& lhs, const LRItem& rhs) {
        return lhs.prod == rhs.prod && lhs.dot == rhs.dot;
    }

    std::ostream& operator<<(std::ostream& os, const LRItem& item) {
        os << "[" << Symbol(item.prod->lhs) << " ->";
        for (size_t i = 0; i < item.prod->rhs.size(); ++i) {
            if (item.dot == i)
                os << " •";
            os << " " << item.prod->rhs[i];
        }

        if (item.is_dot_at_end())
            os << " •";

        return os << ", " << item.lookahead << "]";
    }
}

size_t std::hash<lr::LRItem>::operator()(const lr::LRItem& item) const {
    size_t hash = std::hash<const Production*>{}(item.prod);
    hash = hash_combine(hash, std::hash<size_t>{}(item.dot));
    hash = hash_combine(hash, std::hash<Terminal>{}(item.lookahead));
    return hash;
}
