#include "pareas/llpgen/generator.hpp"
#include "pareas/llpgen/item.hpp"

#include <span>
#include <deque>
#include <ostream>
#include <cstddef>
#include <cassert>

#include <iostream>

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

LLPGenerator::LLPGenerator(const Grammar* g): g(g) {
    this->base_first_sets = this->compute_base_first_or_last_set(true);
    this->base_last_sets = this->compute_base_first_or_last_set(false);

    this->follow_sets = this->compute_follow_or_before_sets(true);
    this->before_sets = this->compute_follow_or_before_sets(false);

    this->item_sets = this->compute_item_sets();
}

void LLPGenerator::dump(std::ostream& os) {
    auto dump_nt_ts = [&](const std::unordered_map<NonTerminal, TerminalSet>& sets){
        for (auto [nt, set] : sets) {
            os << nt << ":\t";
            for (const auto& t : set) {
                os << " " << t;
            }
            os << std::endl;
        }
    };

    os << "Base first sets: " << std::endl;
    dump_nt_ts(this->base_first_sets);

    os << "Base last sets: " << std::endl;
    dump_nt_ts(this->base_last_sets);

    os << "Follow sets: " << std::endl;
    dump_nt_ts(this->follow_sets);

    os << "Before sets: " << std::endl;
    dump_nt_ts(this->before_sets);

    os << "Item sets:" << std::endl;
    for (const auto& set : this->item_sets) {
        set.dump(os);
    }
}

PSLSTable LLPGenerator::build_psls_table() {
    auto psls = PSLSTable();
    for (const auto& set : this->item_sets) {
        for (const auto& item : set.items) {
            if (item.is_dot_at_begin())
                continue;
            const auto& sym = item.sym_before_dot();
            if (!sym.is_terminal)
                continue;
            else if (item.lookahead.is_null() || item.lookback.is_null())
                continue;

            psls.insert({item.lookback, item.lookahead}, item.gamma);
        }
    }

    return psls;
}

LLTable LLPGenerator::build_ll_table() {
    auto ll = LLTable();
    for (const auto& prod : this->g->productions) {
        auto first = this->compute_first_or_last_set(prod.rhs, true);

        bool has_null = false;
        for (const auto& t : first) {
            if (t.is_null()) {
                has_null = true;
                continue;
            }

            ll.insert({prod.lhs, t}, &prod);
        }

        if (has_null) {
            const auto& follow = this->follow_sets[prod.lhs];
            for (const auto& t : follow) {
                ll.insert({prod.lhs, t}, &prod);
            }
        }
    }

    return ll;
}

LLPTable LLPGenerator::build_llp_table(const LLTable& ll, const PSLSTable& psls) {
    auto llp = LLPTable();
    for (const auto& [ap, gamma] : psls.table) {
        auto initial_stack = std::vector<Symbol>(gamma.rbegin(), gamma.rend());
        auto stack = initial_stack;
        auto productions = ll.partial_parse(ap.y, stack);
        llp.table[ap] = {initial_stack, stack, productions};
    }

    return llp;
}

std::unordered_set<ItemSet> LLPGenerator::compute_item_sets() {
    auto sets = std::unordered_set<ItemSet>();
    auto queue = std::deque<ItemSet>();

    auto enqueue = [&](const ItemSet& set) {
        bool inserted = sets.insert(set).second;
        if (inserted) {
            queue.emplace_back(set);
        }
    };

    {
        auto initial = ItemSet();
        initial.items.insert(Item::initial(*this->g));
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

    return sets;
}

std::unordered_map<NonTerminal, TerminalSet> LLPGenerator::compute_base_first_or_last_set(bool first) {
    auto sets = std::unordered_map<NonTerminal, TerminalSet>();

    auto add_prod = [&](const Production& prod) {
        bool changed = false;

        for (size_t i = 0; i < prod.rhs.size(); ++i) {
            const auto& sym = first ? prod.rhs[i] : prod.rhs[prod.rhs.size() - i - 1];

            if (sym.is_null()) {
                continue;
            } else if (sym.is_terminal) {
                changed |= sets[prod.lhs].insert(sym.as_terminal()).second;
                return changed;
            } else {
                auto dst_set = sets[prod.lhs]; // be lazy and copy for now
                const auto& sym_set = sets[sym.as_non_terminal()];
                bool sym_set_has_empty = sym_set.contains(Terminal::null());

                changed |= merge_terminal_sets_omit_null(dst_set, sym_set);
                sets[prod.lhs] = dst_set;

                if (!sym_set_has_empty)
                    return changed;
            }
        }

        changed |= sets[prod.lhs].insert(Terminal::null()).second;
        return changed;
    };

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : this->g->productions) {
            changed |= add_prod(prod);
        }
    }

    return sets;
}

std::unordered_map<NonTerminal, TerminalSet> LLPGenerator::compute_follow_or_before_sets(bool follow) {
    auto sets = std::unordered_map<NonTerminal, TerminalSet>();

    auto add_prod = [&](const Production& prod) {
        bool changed = false;

        for (size_t i = 0; i < prod.rhs.size(); ++i) {
            const auto& sym = prod.rhs[i];
            if (sym.is_terminal)
                continue;
            auto nt = sym.as_non_terminal();

            auto b = follow ?
                std::span(prod.rhs).subspan(i + 1) :
                std::span(prod.rhs).subspan(0, i);

            auto ts = this->compute_first_or_last_set(b, follow);
            auto set = sets[nt]; // copy for now
            changed |= merge_terminal_sets_omit_null(set, ts);

            if (ts.contains(Terminal::null())) {
                changed |= merge_terminal_sets_omit_null(set, sets[prod.lhs]);
            }

            sets[nt] = set;
        }

        return changed;
    };

    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& prod : this->g->productions) {
            changed |= add_prod(prod);
        }
    }

    return sets;
}

TerminalSet LLPGenerator::compute_first_or_last_set(std::span<const Symbol> symbols, bool first) {
    auto set = TerminalSet();
    const auto& base_sets = first ? this->base_first_sets : this->base_last_sets;

    for (size_t i = 0; i < symbols.size(); ++i) {
        const auto& sym = first ? symbols[i] : symbols[symbols.size() - i - 1];
        if (sym.is_null()) {
            continue;
        } if (sym.is_terminal) {
            set.insert(sym.as_terminal());
            return set;
        }

        auto nt = sym.as_non_terminal();
        const auto& ts = base_sets.find(nt);
        assert(ts != base_sets.end());
        merge_terminal_sets_omit_null(set, ts->second);

        if (!ts->second.contains(Terminal::null())) {
            return set;
        }
    }

    set.insert(Terminal::null());
    return set;
}

ItemSet LLPGenerator::predecessor(const ItemSet& set, const Symbol& sym) {
    auto new_set = ItemSet();
    for (const auto& item : set.items) {
        if (item.is_dot_at_begin() || item.sym_before_dot() != sym)
            continue;

        auto alpha = item.syms_before_dot();
        alpha = alpha.subspan(0, alpha.size() - 1);

        auto us = this->compute_first_or_last_set(alpha, false);
        if (us.contains(Terminal::null())) {
            const auto& before = this->before_sets[item.prod->lhs];
            if (!before.empty()) // Special case: Start rule
                us.erase(Terminal::null());
            merge_terminal_sets_omit_null(us, before);
        }

        Symbol xvi[] = {sym, item.lookahead};
        auto vs = this->compute_first_or_last_set(xvi, true);

        for (const auto& u : us) {
            for (const auto& v : vs) {
                auto gamma = this->compute_gamma(v, sym, item.gamma);
                auto new_item = Item{
                    .prod = item.prod,
                    .dot = item.dot - 1,
                    .lookback = u,
                    .lookahead = v,
                    .gamma = gamma,
                };
                new_set.items.insert(new_item);
            }
        }
    }

    return new_set;
}

void LLPGenerator::closure(ItemSet& set) {
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

            auto us = this->compute_first_or_last_set(std::span(prod.rhs), false);
            if (us.contains(Terminal::null())) {
                us.erase(Terminal::null());
                merge_terminal_sets_omit_null(us, this->before_sets[prod.lhs]);
            }

            for (const auto& u : us) {
                enqueue(Item{
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

std::vector<Symbol> LLPGenerator::compute_gamma(const Terminal& v, const Symbol& x, std::span<const Symbol> delta) {
    assert(!x.is_null());
    assert(!v.is_null());

    auto gamma = std::vector<Symbol>();
    if (x.is_terminal) {
        gamma.push_back(x.as_terminal());
        return gamma;
    }

    gamma.push_back(x);
    auto nt = x.as_non_terminal();

    if (!this->base_first_sets[nt].contains(v)) {
        assert(this->base_first_sets[nt].contains(Terminal::null()));
        assert(this->compute_first_or_last_set(delta, true).contains(v));
        gamma.push_back(v);
    }

    return gamma;
}
