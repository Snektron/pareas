#include "pareas/llpgen/ll.hpp"
#include "pareas/llpgen/hash_util.hpp"

namespace ll {
    size_t State::Hash::operator()(const State& key) const {
        return hash_combine(std::hash<NonTerminal>{}(key.stack_top), std::hash<Terminal>{}(key.lookahead));
    }

    bool operator==(const State& lhs, const State& rhs) {
        return lhs.stack_top == rhs.stack_top && lhs.lookahead == rhs.lookahead;
    }

    std::vector<const Production*> ParsingTable::partial_parse(const Terminal& y, std::vector<Symbol>& stack) const {
        // TODO: Convert asserts to errors

        auto productions = std::vector<const Production*>();
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

    void ParsingTable::dump_csv(std::ostream& os) const {
        auto nts = std::unordered_set<NonTerminal>();
        auto ts = std::unordered_set<Terminal>();

        for (const auto& [state, prod] : this->table) {
            const auto& [nt, t] = state;
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

    Generator::Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf):
        er(er), g(g), tsf(tsf) {}

    ParsingTable Generator::build_parsing_table() {
        auto ll = ParsingTable();
        bool error = false;

        auto insert = [&](const State& state, const Production* prod) {
            auto it = ll.table.find(state);
            if (it != ll.table.end()) {
                this->er->error_fmt(prod->loc, "LL parse conflict, grammar is not LL(1)");
                this->er->note(it->second->loc, "Conflicts with this production");

                error = true;
                return;
            }

            ll.table.insert(it, {state, prod});
        };

        for (const auto& prod : this->g->productions) {
            auto first = this->tsf->compute_first(prod.rhs);

            bool has_null = false;
            for (const auto& t : first) {
                if (t.is_null()) {
                    has_null = true;
                    continue;
                }

                insert({prod.lhs, t}, &prod);
            }

            if (has_null) {
                const auto& follow = this->tsf->follow(prod.lhs);
                for (const auto& t : follow) {
                    insert({prod.lhs, t}, &prod);
                }
            }
        }

        if (error)
            throw ConflictError();

        return ll;
    }
}
