#include "llpgen/generator.hpp"
#include "llpgen/item.hpp"
#include "llpgen/configuration.hpp"

#include <span>
#include <ostream>
#include <iostream>
#include <cstddef>
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

LLPGenerator::LLPGenerator(const Grammar* g): g(g) {
    this->base_first_sets = this->compute_base_first_or_last_set(true);
    this->base_last_sets = this->compute_base_first_or_last_set(false);

    this->follow_sets = this->compute_follow_or_before_sets(true);
    this->before_sets = this->compute_follow_or_before_sets(false);
}

void LLPGenerator::dump(std::ostream& os) {
    auto dump_nt_ts = [&](const std::unordered_map<NonTerminal, TerminalSet>& sets){
        for (auto [nt, set] : sets) {
            os << "    " << nt << ":\t";
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
        if (sym.is_terminal) {
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

