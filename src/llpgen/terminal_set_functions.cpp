#include "pareas/llpgen/terminal_set_functions.hpp"

#include <fmt/ostream.h>

#include <cassert>

namespace {
    using pareas::TerminalSet;

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

namespace pareas {
    TerminalSetFunctions::TerminalSetFunctions(const Grammar& g) {
        this->base_first_sets = this->compute_base_first_or_last_set(g, true);
        this->base_last_sets = this->compute_base_first_or_last_set(g, false);

        this->follow_sets = this->compute_follow_or_before_sets(g, true);
        this->before_sets = this->compute_follow_or_before_sets(g, false);
    }

    const TerminalSet& TerminalSetFunctions::first(const NonTerminal& nt) const {
        auto it = this->base_first_sets.find(nt);
        assert(it != this->base_first_sets.end());
        return it->second;
    }

    const TerminalSet& TerminalSetFunctions::last(const NonTerminal& nt) const {
        auto it = this->base_last_sets.find(nt);
        assert(it != this->base_last_sets.end());
        return it->second;
    }

    const TerminalSet& TerminalSetFunctions::follow(const NonTerminal& nt) const {
        auto it = this->follow_sets.find(nt);
        assert(it != this->follow_sets.end());
        return it->second;
    }

    const TerminalSet& TerminalSetFunctions::before(const NonTerminal& nt) const {
        auto it = this->before_sets.find(nt);
        assert(it != this->before_sets.end());
        return it->second;
    }

    TerminalSet TerminalSetFunctions::compute_first(std::span<const Symbol> symbols) const {
        return this->compute_first_or_last_set(symbols, true);
    }

    TerminalSet TerminalSetFunctions::compute_last(std::span<const Symbol> symbols) const {
        return this->compute_first_or_last_set(symbols, false);
    }

    void TerminalSetFunctions::dump(std::ostream& os) {
        auto dump_nt_ts = [&](const std::unordered_map<NonTerminal, TerminalSet>& sets){
            for (auto [nt, set] : sets) {
                fmt::print(os, "    {}:\t", nt);
                for (const auto& t : set) {
                    fmt::print(os, " {}", t);
                }
                fmt::print(os, "\n");
            }
        };

        fmt::print(os, "Base first sets:\n");
        dump_nt_ts(this->base_first_sets);

        fmt::print(os, "Base last sets:\n");
        dump_nt_ts(this->base_last_sets);

        fmt::print(os, "Follow sets:\n");
        dump_nt_ts(this->follow_sets);

        fmt::print(os, "Before sets:\n");
        dump_nt_ts(this->before_sets);
    }


    TerminalSetMap TerminalSetFunctions::compute_base_first_or_last_set(const Grammar& g, bool first) const {
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
            for (const auto& prod : g.productions) {
                changed |= add_prod(prod);
            }
        }

        return sets;
    }

    TerminalSetMap TerminalSetFunctions::compute_follow_or_before_sets(const Grammar& g, bool follow) const {
        auto sets = std::unordered_map<NonTerminal, TerminalSet>();

        // Pre-insert left-hand sides: this will both make sure that a key is allocated
        // (allowing us to omit a copy), and will make sure that any LHS with empty follow/
        // before set is present
        for (const auto& prod : g.productions) {
            sets.insert({prod.lhs, {}});
        }

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
                auto it = sets.find(nt);

                // Should be pre-inserted.  If not, there is no production which has this
                // symbol as LHS.
                // TODO: Report error for this situation.
                assert(it != sets.end());

                changed = merge_terminal_sets_omit_null(it->second, ts);
                if (ts.contains(Terminal::null())) {
                    changed |= merge_terminal_sets_omit_null(it->second, sets[prod.lhs]);
                }
            }

            return changed;
        };

        bool changed = true;
        while (changed) {
            changed = false;

            for (const auto& prod : g.productions) {
                changed |= add_prod(prod);
            }
        }

        return sets;
    }

    TerminalSet TerminalSetFunctions::compute_first_or_last_set(std::span<const Symbol> symbols, bool first) const {
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
}
