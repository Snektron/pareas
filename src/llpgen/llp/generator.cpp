#include "pareas/llpgen/llp/generator.hpp"

#include <deque>
#include <algorithm>
#include <cassert>

namespace {
    bool merge_terminal_sets_omit_null(TerminalSet& dst, const TerminalSet& src) {
        bool changed = false;
        for (const auto& t : src) {
            if (t.is_null())
                continue;
            changed |= dst.insert(t).second;
        }

        return changed;
    }
}

namespace llp {
    Generator::Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf):
        er(er), g(g), tsf(tsf) {}

    PSLSTable Generator::build_psls_table() {
        this->compute_item_sets();

        auto psls = PSLSTable();
        bool error = false;

        auto insert = [&](const Item& item) {
            auto ap = AdmissiblePair{item.lookback, item.lookahead};
            auto it = psls.table.find(ap);

            if (it == psls.table.end()) {
                psls.table.insert(it, {ap, {item.gamma, item.prod}});
                return;
            }

            const auto& gamma = it->second.gamma;
            if (std::equal(gamma.begin(), gamma.end(), item.gamma.begin(), item.gamma.end()))
                return;

            this->er->error_fmt(item.prod->loc, "PSLS conflict between terminals '", ap.x, "' and '", ap.y, "', grammar is not LLP(1, 1)");

            if (it->second.prod != item.prod)
                this->er->note(it->second.prod->loc, "Conflicts with this production");

            error = true;
        };

        for (const auto& set : this->item_sets) {
            for (const auto& item : set.items) {
                if (item.is_dot_at_begin())
                    continue;
                const auto& sym = item.sym_before_dot();
                if (!sym.is_terminal)
                    continue;
                else if (item.lookahead.is_null() || item.lookback.is_null())
                    continue;

                insert(item);
            }
        }

        if (error)
            throw PSLSConflictError();

        return psls;
    }

    ParsingTable Generator::build_parsing_table(const ll::ParsingTable& ll, const PSLSTable& psls) {
        auto llp = ParsingTable();

        {
            auto initial_stack = std::vector<Symbol>({this->g->start->lhs});
            auto stack = initial_stack;
            auto productions = ll.partial_parse(this->g->left_delim, stack);
            llp.start = {initial_stack, stack, productions};
        }

        for (const auto& [ap, entry] : psls.table) {
            auto initial_stack = std::vector<Symbol>(entry.gamma.rbegin(), entry.gamma.rend());
            auto stack = initial_stack;
            auto productions = ll.partial_parse(ap.y, stack);
            llp.table[ap] = {initial_stack, stack, productions};
        }

        return llp;
    }

    void Generator::dump(std::ostream& os) {
        os << "Item sets:" << std::endl;
        for (const auto& set : this->item_sets) {
            set.dump(os);
        }
    }

    void Generator::compute_item_sets() {
        if (!this->item_sets.empty())
            return; // Already computed

        auto queue = std::deque<LLPItemSet>();

        auto enqueue = [&](const LLPItemSet& set) {
            bool inserted = this->item_sets.insert(set).second;
            if (inserted) {
                queue.emplace_back(set);
            }
        };

        {
            auto initial = LLPItemSet();
            initial.items.insert({
                .prod = this->g->start,
                .dot = this->g->start->rhs.size(),
                .lookback = this->g->right_delim,
                .lookahead = Terminal::null(),
                .gamma = {},
            });

            enqueue(initial);
        }

        while (!queue.empty()) {
            auto set = queue.front();
            queue.pop_front();

            auto syms = set.syms_before_dots();
            for (const auto& sym : syms) {
                auto new_set = this->predecessor(set, sym);
                this->closure(new_set);
                enqueue(new_set);
            }
        }
    }

    LLPItemSet Generator::predecessor(const LLPItemSet& set, const Symbol& sym) {
        auto new_set = LLPItemSet();
        for (const auto& item : set.items) {
            if (item.is_dot_at_begin() || item.sym_before_dot() != sym)
                continue;

            auto alpha = item.syms_before_dot();
            alpha = alpha.subspan(0, alpha.size() - 1);

            auto us = this->tsf->compute_last(alpha);
            if (us.contains(Terminal::null())) {
                const auto& before = this->tsf->before(item.prod->lhs);
                if (!before.empty()) // Special case: Start rule
                    us.erase(Terminal::null());
                merge_terminal_sets_omit_null(us, before);
            }

            Symbol xvi[] = {sym, item.lookahead};
            auto vs = this->tsf->compute_first(xvi);

            for (const auto& u : us) {
                for (const auto& v : vs) {
                    auto gamma = this->compute_gamma(v, sym, item.gamma);
                    new_set.items.insert({
                        .prod = item.prod,
                        .dot = item.dot - 1,
                        .lookback = u,
                        .lookahead = v,
                        .gamma = gamma,
                    });
                }
            }
        }

        return new_set;
    }

    void Generator::closure(LLPItemSet& set) {
        auto queue = std::deque<Item>();

        auto enqueue = [&](const Item& item) {
            bool inserted = set.items.insert(item).second;
            if (item.is_dot_at_begin() || item.sym_before_dot().is_terminal || !inserted)
                return;
            queue.push_back(item);
        };

        for (const auto& item : set.items) {
            if (!item.is_dot_at_begin() && !item.sym_before_dot().is_terminal)
                queue.push_back(item);
        }

        while (!queue.empty()) {
            auto item = queue.front();
            queue.pop_front();

            // These conditions are guaranteed before inserting an item into the queue.
            auto nt = item.sym_before_dot().as_non_terminal();

            for (const auto& prod : this->g->productions) {
                if (prod.lhs != nt)
                    continue;

                auto us = this->tsf->compute_last(std::span(prod.rhs));
                if (us.contains(Terminal::null())) {
                    us.erase(Terminal::null());
                    merge_terminal_sets_omit_null(us, this->tsf->before(prod.lhs));
                }

                for (const auto& u : us) {
                    enqueue({
                        .prod = &prod,
                        .dot = prod.rhs.size(),
                        .lookback = u,
                        .lookahead = item.lookahead,
                        .gamma = item.gamma,
                    });
                }
            }
        }
    }

    std::vector<Symbol> Generator::compute_gamma(const Terminal& v, const Symbol& x, std::span<const Symbol> delta) {
        assert(!x.is_null());
        assert(!v.is_null());

        auto gamma = std::vector<Symbol>();
        if (x.is_terminal) {
            gamma.push_back(x.as_terminal());
            return gamma;
        }

        gamma.push_back(x);
        auto nt = x.as_non_terminal();

        const auto& first = this->tsf->first(nt);
        if (!first.contains(v)) {
            assert(first.contains(Terminal::null()));
            assert(this->tsf->compute_first(delta).contains(v));
            gamma.push_back(v);
        }

        return gamma;
    }
}
