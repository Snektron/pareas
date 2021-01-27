#include "pareas/llpgen/psls_table.hpp"

#include <unordered_set>
#include <ostream>

PSLSConflictError::PSLSConflictError(const AdmissiblePair& ap, std::span<const Symbol> a, std::span<const Symbol> b):
    InvalidGrammarError("PSLS conflict: Grammar is not LLP(1, 1)"),
    ap(ap), a(a.begin(), a.end()), b(b.begin(), b.end()) {}

void PSLSTable::insert(const AdmissiblePair& ap, std::span<const Symbol> symbols) {
    auto it = this->table.find(ap);
    if (it == this->table.end()) {
        auto sym_vec = std::vector<Symbol>(symbols.begin(), symbols.end());
        this->table.insert(it, {ap, std::move(sym_vec)});
        return;
    }

    const auto& existing_syms = it->second;
    if (!std::equal(existing_syms.begin(), existing_syms.end(), symbols.begin(), symbols.end()))
        throw PSLSConflictError(ap, existing_syms, symbols);
}

void PSLSTable::dump_csv(std::ostream& os) const {
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

    auto ys = std::unordered_set<Terminal>();
    auto xs = std::unordered_set<Terminal>();

    for (const auto& [ap, gamma] : this->table) {
        const auto& [x, y] = ap;
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
