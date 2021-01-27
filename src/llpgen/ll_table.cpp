#include "pareas/llpgen/ll_table.hpp"
#include "pareas/llpgen/hash_util.hpp"

#include <unordered_set>
#include <ostream>
#include <cassert>

LLConflictError::LLConflictError(const LLTableKey& a, const LLTableKey& b):
    InvalidGrammarError("LL conflict: Grammar is not LL(1)"),
    a(a), b(b) {}

bool operator==(const LLTableKey& lhs, const LLTableKey& rhs) {
    return lhs.nt == rhs.nt && lhs.t == rhs.t;
}

size_t std::hash<LLTableKey>::operator()(const LLTableKey& key) const {
    return hash_combine(std::hash<NonTerminal>{}(key.nt), std::hash<Terminal>{}(key.t));
}

void LLTable::insert(const LLTableKey& key, const Production* prod) {
    auto it = this->table.find(key);
    if (it != this->table.end())
        throw LLConflictError(it->first, key);

    this->table.insert(it, {key, prod});
}

std::vector<const Production*> LLTable::partial_parse(const Terminal& y, std::vector<Symbol>& stack) const {
    auto productions = std::vector<const Production*>();

    // TODO: Convert asserts to errors

    while (true) {
        assert(!stack.empty());
        auto top = stack.back();
        stack.pop_back();

        if (top.is_null()) {
            continue;
        } else if (top.is_terminal) {
            assert(y == top.as_terminal());
            break;
        }

        auto nt = top.as_non_terminal();

        auto it = this->table.find({nt, y});
        assert(it != this->table.end());
        productions.push_back(it->second);
        const auto& to_push = it->second->rhs;

        stack.insert(stack.end(), to_push.rbegin(), to_push.rend());
    }

    return productions;
}

void LLTable::dump_csv(std::ostream& os) const {
    auto nts = std::unordered_set<NonTerminal>();
    auto ts = std::unordered_set<Terminal>();

    for (const auto& [key, prod] : this->table) {
        const auto& [nt, t] = key;
        nts.insert(nt);
        ts.insert(t);
    }

    for (const auto& t : ts) {
        os << "," << t;
    }
    os << std::endl;

    for (const auto& nt : nts) {
        os << nt;
        // Hope that this iterates in the same order
        for (const auto& t : ts) {
            os << ",";
            auto it = this->table.find({nt, t});
            if (it == this->table.end())
                continue;

            os << '"' << *it->second << '"';
        }
        os << std::endl;
    }
}
