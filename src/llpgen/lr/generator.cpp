#include "pareas/llpgen/lr/generator.hpp"

#include <deque>
#include <ostream>
#include <cassert>

namespace lr {
    Generator::Generator(const Grammar* g, const TerminalSetFunctions* tsf):
        g(g), tsf(tsf) {}

    ParsingTable Generator::build_parsing_table() {
        auto lr_table = ParsingTable();
        auto queue = std::deque<LRItemSet>();

        bool error = false;

        auto enqueue = [&](const LRItemSet& set) {
            auto id = this->item_sets.size();
            auto [it, inserted] = this->item_sets.insert({set, id});
            if (inserted) {
                queue.emplace_back(set);
            }
            return it->second;
        };

        auto insert_reductions = [&](const LRItemSet& set, size_t state) {
            for (const auto& item : set.items) {
                if (!item.is_dot_at_end())
                    continue;

                error |= !lr_table.insert_action({state, item.lookahead}, Action::reduce(item.prod));
            }
        };

        auto insert_successor = [&](const LRItemSet& set, size_t state, size_t successor, const Symbol& sym) {
            if (sym.is_terminal)
                error |= !lr_table.insert_action({state, sym.as_terminal()}, Action::shift(successor));
            else
                error |= !lr_table.insert_goto({state, sym.as_non_terminal()}, successor);
        };

        {
            auto initial = LRItemSet();
            initial.items.insert({
                .prod = this->g->start,
                .dot = 0,
                .lookahead = this->g->right_delim,
            });
            this->closure(initial);
            enqueue(initial);
        }

        while (!queue.empty()) {
            auto set = queue.front();
            queue.pop_front();

            auto it = this->item_sets.find(set);
            assert(it != this->item_sets.end());
            size_t state = it->second;

            insert_reductions(set, state);

            auto syms = set.syms_after_dots();
            for (const auto& sym : syms) {
                auto new_set = this->successor(set, sym);
                this->closure(new_set);
                auto succ = enqueue(new_set);
                insert_successor(set, state, succ, sym);
            }
        }

        if (error) {
            // TODO: Generate error message
            throw ConflictError();
        }

        return lr_table;
    }

    void Generator::dump(std::ostream& os) const {
        os << "Item sets:" << std::endl;
        for (const auto& [set, id] : this->item_sets) {
            os << "Set " << id << ":" << std::endl;
            set.dump(os);
        }
    }

    LRItemSet Generator::successor(const LRItemSet& set, const Symbol& sym) {
        auto new_set = LRItemSet();

        for (const auto& item : set.items) {
            if (item.is_dot_at_end() || item.sym_after_dot() != sym)
                continue;

            new_set.items.insert({
                .prod = item.prod,
                .dot = item.dot + 1,
                .lookahead = item.lookahead,
            });
        }

        return new_set;
    }

    void Generator::closure(LRItemSet& set) {
        auto queue = std::deque<Item>();

        auto enqueue = [&](const Item& item) {
            bool inserted = set.items.insert(item).second;
            if (item.is_dot_at_end() || item.sym_after_dot().is_terminal || !inserted)
                return;
            queue.push_back(item);
        };

        for (const auto& item : set.items) {
            if (!item.is_dot_at_end() && !item.sym_after_dot().is_terminal)
                queue.push_back(item);
        }

        while (!queue.empty()) {
            auto item = queue.front();
            queue.pop_front();

            // These conditions are guaranteed before inserting an item into the queue.
            auto nt = item.sym_after_dot().as_non_terminal();

            for (const auto& prod : this->g->productions) {
                if (prod.lhs != nt)
                    continue;

                auto v = item.syms_after_dot().subspan(1);

                auto us = this->tsf->compute_first(v);
                if (us.contains(Terminal::null())) {
                    us.erase(Terminal::null());
                    us.insert(item.lookahead);
                }

                for (const auto& u : us) {
                    enqueue({
                        .prod = &prod,
                        .dot = 0,
                        .lookahead = u,
                    });
                }
            }
        }
    }
}
