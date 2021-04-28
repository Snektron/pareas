#include "pareas/lpg/lexer/fsa.hpp"
#include "pareas/lpg/lexer/regex.hpp"
#include "pareas/lpg/lexer/lexical_grammar.hpp"
#include "pareas/lpg/escape.hpp"
#include "pareas/lpg/hash_util.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <limits>
#include <cassert>

using Symbol = pareas::lexer::FiniteStateAutomaton::Symbol;
using StateIndex = pareas::lexer::FiniteStateAutomaton::StateIndex;

namespace {
    using namespace pareas::lexer;

    struct StateSet {
        std::unordered_set<StateIndex> states;

        struct Hash {
            size_t operator()(const StateSet& ss) const;
        };
    };

    size_t StateSet::Hash::operator()(const StateSet& ss) const {
        // Need to make sure that the order doesn't matter for this hash.
        return pareas::hash_order_independent_range(ss.states.begin(), ss.states.end(), std::hash<StateIndex>{});
    }

    bool operator==(const StateSet& lhs, const StateSet& rhs) {
        return lhs.states == rhs.states;
    }

    void closure(const FiniteStateAutomaton& fsa, StateSet& ss) {
        auto queue = std::deque<StateIndex>();
        for (auto state : ss.states)
            queue.push_back(state);

        auto enqueue = [&](StateIndex state) {
            auto inserted = ss.states.insert(state).second;
            if (inserted)
                queue.push_back(state);
        };

        while (!queue.empty()) {
            auto src = queue.front();
            queue.pop_front();

            for (const auto& [maybe_sym, dst, _] : fsa[src].transitions) {
                if (!maybe_sym.has_value())
                    enqueue(dst);
            }
        }
    }

    std::unordered_set<Symbol> follow(const FiniteStateAutomaton& fsa, const StateSet& ss) {
        // Epsilon-transitions should already be dealt with at this point
        auto syms = std::unordered_set<Symbol>();
        for (auto src : ss.states) {
            for (const auto& [maybe_sym, dst, _] : fsa[src].transitions) {
                if (maybe_sym.has_value())
                    syms.insert(maybe_sym.value());
            }
        }
        return syms;
    }

    StateSet move(const FiniteStateAutomaton& fsa, const StateSet& ss, Symbol move_sym) {
        auto after_move = StateSet{};
        for (auto src : ss.states) {
            for (const auto& [sym, dst, _] : fsa[src].transitions) {
                if (sym == move_sym)
                    after_move.states.insert(dst);
            }
        }

        return after_move;
    }
}

namespace pareas::lexer {
    FiniteStateAutomaton::FiniteStateAutomaton() {
        assert(this->add_state() == REJECT);
        assert(this->add_state() == START);
    }

    size_t FiniteStateAutomaton::num_states() const {
        return this->states.size();
    }

    StateIndex FiniteStateAutomaton::add_state() {
        StateIndex index = this->num_states();
        this->states.push_back({
            .lexeme = nullptr,
            .transitions = {}
        });
        return index;
    }

    void FiniteStateAutomaton::add_transition(StateIndex src, StateIndex dst, std::optional<uint8_t> sym, bool produces_lexeme) {
        assert(src < this->num_states());
        assert(dst < this->num_states());

        this->states[src].transitions.push_back({sym, dst, produces_lexeme});
    }

    void FiniteStateAutomaton::add_epsilon_transition(StateIndex src, StateIndex dst, bool produces_lexeme) {
        this->add_transition(src, dst, std::nullopt, produces_lexeme);
    }

    std::optional<StateIndex> FiniteStateAutomaton::find_first_transition_dst(StateIndex src, std::optional<uint8_t> sym) const {
        for (const auto t : this->states[src].transitions) {
            if (t.maybe_sym == sym) {
                return t.dst;
            }
        }

        return std::nullopt;
    }

    auto FiniteStateAutomaton::operator[](StateIndex state) -> State& {
        assert(state < this->num_states());
        return this->states[state];
    }

    auto FiniteStateAutomaton::operator[](StateIndex state) const -> const State& {
        assert(state < this->num_states());
        return this->states[state];
    }

    void FiniteStateAutomaton::dump_dot(std::ostream &os) const {
        fmt::print(os, "digraph {{\n");
        fmt::print(os, "    start [style=invis];\n");
        fmt::print(os, "    start -> state{};\n", START);

        for (size_t src = 0; src < this->num_states(); ++src) {
            const auto& [lexeme, transitions] = this->states[src];
            fmt::print(
                os,
                "    state{} [shape=\"{}\", label=\"{}\"];\n",
                src,
                lexeme ? "doublecircle" : "circle",
                lexeme ? lexeme->name : ""
            );

            for (const auto& [maybe_sym, dst, produces_lexeme] : transitions) {
                auto style = produces_lexeme ? ", color=blue" : "";

                if (maybe_sym.has_value()) {
                    fmt::print(os, "    state{} -> state{} [label=\"{:q}\"{}];\n", src, dst, EscapeFormatter{maybe_sym.value()}, style);
                } else {
                    fmt::print(os, "    state{} -> state{} [label=\"Ɛ\"{}];\n", src, dst, style);
                }
            }
        }

        fmt::print(os, "}}\n");
    }

    void FiniteStateAutomaton::to_dfa(const LexicalGrammar* g, FiniteStateAutomaton& dfa, StateIndex nfa_start, StateIndex dfa_start) const {
        auto seen = std::unordered_map<StateSet, StateIndex, StateSet::Hash>();
        auto queue = std::deque<StateSet>();

        auto enqueue = [&](const StateSet& ss) {
            auto it = seen.find(ss);
            if (it != seen.end())
                return it->second;

            auto state = dfa.add_state();
            seen.insert(it, {ss, state});
            queue.push_back(ss);
            return state;
        };

        {
            auto start_ss = StateSet{{nfa_start}};
            // Move over all epsilon-transitions
            closure(*this, start_ss);

            seen.insert({start_ss, dfa_start});
            queue.push_back(start_ss);
        }

        while (!queue.empty()) {
            auto ss = queue.front();
            auto src = seen[ss];
            queue.pop_front();
            auto follow_set = follow(*this, ss);

            for (auto sym : follow_set) {
                auto new_ss = move(*this, ss, sym);
                // Move over all epsilon-transitions
                closure(*this, new_ss);
                auto dst = enqueue(new_ss);
                dfa.add_transition(src, dst, sym);
            }
        }

        for (const auto& [ss, dfa_index] : seen) {
            auto& dfa_state = dfa[dfa_index];

            for (auto nfa_index : ss.states) {
                const auto& nfa_state = this->states[nfa_index];

                if (nfa_state.lexeme && dfa_state.lexeme) {
                    if (g->lexeme_id(nfa_state.lexeme) < g->lexeme_id(dfa_state.lexeme)) {
                        dfa_state.lexeme = nfa_state.lexeme;
                    }
                } else if (nfa_state.lexeme) {
                    dfa_state.lexeme = nfa_state.lexeme;
                }
            }
        }
    }

    FiniteStateAutomaton FiniteStateAutomaton::build_lexer_dfa(const LexicalGrammar* g) {
        auto nfa = FiniteStateAutomaton();

        auto succ_nfa_roots = std::unordered_map<const Lexeme*, StateIndex>();
        for (const auto& lexeme : g->lexemes) {
            auto regex_start = nfa.add_state();
            auto regex_end = lexeme.regex->compile(nfa, regex_start);
            nfa.states[regex_end].lexeme = &lexeme;

            if (lexeme.preceded_by.empty()) {
                // If there is no list of lexemes this lexeme should be preceded by, it is connected
                // to the start state.
                nfa.add_epsilon_transition(START, regex_start);
                continue;
            }

            // Otherwise, add a new root for each of the 'preceeded by' tokens for this lexeme.
            // We are going to add it to each of them separately to avoid conflicts with
            // lexical grammars like:
            //
            // a = /a/
            // b = /b/
            // c = /c/
            // d = /d/ [a, b]
            // e = /e/ [b, c]

            for (const auto* prec : lexeme.preceded_by) {
                auto it = succ_nfa_roots.find(prec);
                if (it == succ_nfa_roots.end()) {
                    it = succ_nfa_roots.insert({prec, nfa.add_state()}).first;
                }

                auto root = it->second;
                nfa.add_epsilon_transition(root, regex_start);
            }
        }

        // Convert the NFA into a DFA, for each root (including start).
        auto dfa = FiniteStateAutomaton();

        // First do the nfa start state. This should attach to the DFA's start state.
        nfa.to_dfa(g, dfa, START, START);

        // Handle each of the successor roots.
        auto succ_dfa_roots = std::unordered_map<const Lexeme*, StateIndex>();
        for (const auto [lexeme, nfa_root] : succ_nfa_roots) {
            auto dfa_root = dfa.add_state();
            succ_dfa_roots.insert({lexeme, dfa_root});

            nfa.to_dfa(g, dfa, nfa_root, dfa_root);
        }

        // Now its time to add the lexer loop. For each symbol of each final state that does
        // not already have an outgoing transition, add a new transition by looking up where
        // it goes from the start state. If the lexeme in this final state appears in succ_dfa_roots,
        // look up where it goes from there instead.

        for (size_t src = 0; src < dfa.num_states(); ++src) {
            auto& state = dfa.states[src];
            if (!state.lexeme)
                continue;

            // Empty tokens are not allowed, as this could require the lexer to generate two tokens on a transition.
            // It should be filtered out using LexicalGrammar::validate.
            assert(src != START);

            auto outgoing = std::bitset<MAX_SYM + 1>();
            for (auto& t : state.transitions) {
                assert(t.maybe_sym.has_value()); // Not a DFA.
                assert(!outgoing.test(t.maybe_sym.value())); // Not a DFA.
                outgoing.set(t.maybe_sym.value());
            }

            for (size_t sym = 0; sym < outgoing.size(); ++sym) {
                if (outgoing.test(sym))
                    continue;

                // Add a successor edge if required.
                auto it = succ_dfa_roots.find(state.lexeme);
                if (it != succ_dfa_roots.end()) {
                    // Look up where it goes from the successor root.
                    auto root = it->second;
                    auto maybe_dst = dfa.find_first_transition_dst(root, sym);
                    if (maybe_dst.has_value()) {
                        dfa.add_transition(src, maybe_dst.value(), sym, true);
                        continue;
                    }
                }

                // If there was no successor edge, try the start node.

                auto maybe_dst = dfa.find_first_transition_dst(START, sym);
                if (maybe_dst.has_value()) {
                    dfa.add_transition(src, maybe_dst.value(), sym, true);
                    continue;
                }

                // If neither such transition exists, the dfa would end up in a reject state
                // after this symbol. In order to correctly generate the invalid token, add
                // an explicit arc to the reject state, which produces a token.

                dfa.add_transition(src, REJECT, sym, true);
            }
        }

        return dfa;
    }
}
