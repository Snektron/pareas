#ifndef _PAREAS_LLPGEN_LR_LR_TABLE_CPP
#define _PAREAS_LLPGEN_LR_LR_TABLE_CPP

#include "pareas/llpgen/lr/lr_table.hpp"
#include "pareas/llpgen/hash_util.hpp"

#include <ostream>
#include <unordered_set>
#include <algorithm>
#include <cassert>

namespace lr {
    Action Action::shift(size_t state) {
        return {
            .type = Type::SHIFT,
            .value = {.state = state},
        };
    }

    Action Action::reduce(const Production* reduction) {
        return {
            .type = Type::REDUCE,
            .value = {.reduction = reduction},
        };
    }

    size_t Action::as_shift() const {
        assert(this->type == Type::SHIFT);
        return this->value.state;
    }

    const Production* Action::as_reduce() const {
        assert(this->type == Type::REDUCE);
        return this->value.reduction;
    }

    std::ostream& operator<<(std::ostream& os, const Action& action) {
        if (action.type == Action::Type::SHIFT)
            return os << "shift " << action.as_shift();
        return os << "reduce " << action.as_reduce()->tag;
    }

    size_t LRActionKey::Hash::operator()(const LRActionKey& key) const {
        return hash_combine(std::hash<size_t>{}(key.state), std::hash<Terminal>{}(key.lookahead));
    }

    bool operator==(const LRActionKey& lhs, const LRActionKey& rhs) {
        return lhs.state == rhs.state && lhs.lookahead == rhs.lookahead;
    }

    size_t LRGotoKey::Hash::operator()(const LRGotoKey& key) const {
        return hash_combine(std::hash<size_t>{}(key.state), std::hash<NonTerminal>{}(key.nt));
    }

    bool operator==(const LRGotoKey& lhs, const LRGotoKey& rhs) {
        return lhs.state == rhs.state && lhs.nt == rhs.nt;
    }

    bool LRTable::insert_action(const LRActionKey& key, const Action& action) {
        auto it = this->action_table.find(key);
        if (it == this->action_table.end()) {
            this->action_table.insert(it, {key, action});
            return true;
        }

        return false;
    }

    bool LRTable::insert_goto(const LRGotoKey& key, size_t state) {
        auto it = this->goto_table.find(key);
        if (it == this->goto_table.end()) {
            this->goto_table.insert(it, {key, state});
            return true;
        }

        return false;
    }

    void LRTable::dump_csv(std::ostream& os) const {
        size_t states = 0;
        auto ts = std::unordered_set<Terminal>();
        auto nts = std::unordered_set<NonTerminal>();

        for (const auto& [key, action] : this->action_table) {
            ts.insert(key.lookahead);
            states = std::max(key.state + 1, states);
        }

        for (const auto& [key, state] : this->goto_table) {
            nts.insert(key.nt);
            states = std::max(key.state + 1, states);
        }

        for (const auto& t : ts)
            os << ',' << t;
        os << ",|";
        for (const auto& nt : nts)
            os << ',' << nt;
        os << std::endl;

        for (size_t i = 0; i < states; ++i) {
            os << i;

            // Hope that this iterates in the same order
            for (const auto& t : ts) {
                os << ",";
                auto it = this->action_table.find({i, t});
                if (it == this->action_table.end())
                    continue;
                os << '"' << it->second << '"';
            }

            os << ",|";

            for (const auto& nt : nts) {
                os << ",";
                auto it = this->goto_table.find({i, nt});
                if (it == this->goto_table.end())
                    continue;
                os << '"' << it->second << '"';
            }
            os << std::endl;
        }
    }
}

#endif
