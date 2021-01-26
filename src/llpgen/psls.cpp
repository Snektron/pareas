#include "llpgen/psls.hpp"
#include "llpgen/hash_util.hpp"

#include <unordered_set>
#include <stdexcept>
#include <ostream>

bool operator==(const AdmissiblePair& lhs, const AdmissiblePair& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

size_t std::hash<AdmissiblePair>::operator()(const AdmissiblePair& terms) const {
    auto hasher = std::hash<Terminal>{};
    return hash_combine(hasher(terms.x), hasher(terms.y));
}

void PSLSTable::insert(const AdmissiblePair& terms, std::span<const Symbol> symbols) {
    auto it = this->table.find(terms);
    if (it == this->table.end()) {
        auto sym_vec = std::vector<Symbol>(symbols.begin(), symbols.end());
        this->table.insert(it, {terms, std::move(sym_vec)});
        return;
    }

    const auto& existing_syms = it->second;
    if (!std::equal(existing_syms.begin(), existing_syms.end(), symbols.begin(), symbols.end())) {
        throw std::runtime_error("Parse conflict");
    }
}

void PSLSTable::dump_csv(std::ostream& os) const {
    auto ys = std::unordered_set<Terminal>();
    auto xs = std::unordered_set<Terminal>();

    auto dump_syms = [&](const auto& syms) {
        // Custom print of symbols since we need to handle csv escapes
        os << '"';
        bool first = true;
        for (const auto& sym : syms) {
            if (first)
                first = false;
            else
                os << " ";

            os << sym.name;
        }

        os << '"';
    };

    for (const auto& [admissible_pair, gamma] : this->table) {
        const auto& [x, y] = admissible_pair;
        xs.insert(x);
        ys.insert(y);
    }

    for (const auto& y : ys) {
        os << "," << y;
    }
    os << std::endl;

    for (const auto& x : xs) {
        os << x;
        // Hope that this iterates in the same order
        for (const auto& y : ys) {
            os << ",";
            auto it = this->table.find({x, y});
            if (it == this->table.end())
                continue;

            dump_syms(it->second);
        }
        os << std::endl;
    }
}
