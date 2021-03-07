#include "pareas/lpg/lexer/fsa.hpp"
#include "pareas/lpg/lexer/regex.hpp"
#include "pareas/lpg/lexer/lexer_parser.hpp"
#include "pareas/lpg/escape.hpp"
#include "pareas/lpg/hash_util.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <deque>
#include <unordered_map>
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
            .token = nullptr,
            .transitions = {}
        });
        return index;
    }

    void FiniteStateAutomaton::add_transition(StateIndex src, StateIndex dst, std::optional<uint8_t> sym, bool produces_token) {
        assert(src < this->num_states());
        assert(dst < this->num_states());

        this->states[src].transitions.push_back({sym, dst, produces_token});
    }

    void FiniteStateAutomaton::add_epsilon_transition(StateIndex src, StateIndex dst) {
        this->add_transition(src, dst, std::nullopt);
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
            const auto& [token, transitions] = this->states[src];
            fmt::print(
                "    state{} [shape=\"{}\", label=\"{}\"];\n",
                src,
                token ? "doublecircle" : "circle",
                token ? token->name : ""
            );

            for (const auto& [maybe_sym, dst, produces_token] : transitions) {
                auto style = produces_token ? ", color=blue" : "";

                if (maybe_sym.has_value()) {
                    fmt::print("    state{} -> state{} [label=\"{:q}\"{}];\n", src, dst, EscapeFormatter{maybe_sym.value()}, style);
                } else {
                    fmt::print("    state{} -> state{} [label=\"Ɛ\"{}];\n", src, dst, style);
                }
            }
        }

        fmt::print(os, "}}\n");
    }

    FiniteStateAutomaton FiniteStateAutomaton::to_dfa() const {
        auto dfa = FiniteStateAutomaton();
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
            auto start_ss = StateSet{{START}};
            // Move over all epsilon-transitions
            closure(*this, start_ss);

            seen.insert({start_ss, START});
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

                if (nfa_state.token && dfa_state.token) {
                    assert(nfa_state.token->priority != dfa_state.token->priority);

                    if (nfa_state.token->priority < dfa_state.token->priority) {
                        dfa_state.token = nfa_state.token;
                    }
                } else if (nfa_state.token) {
                    dfa_state.token = nfa_state.token;
                }
            }
        }

        return dfa;
    }

    void FiniteStateAutomaton::add_lexer_loop() {
        for (size_t src = 0; src < this->num_states(); ++src) {
            auto& state = this->states[src];
            if (!state.token)
                continue;

            // For all characters that aren't in an outgoing edge of the final state, add
            // a new edge by looking up where it goes from the start node.

            auto outgoing = std::bitset<std::numeric_limits<Symbol>::max() + 1>();
            for (auto& t : state.transitions) {
                assert(t.maybe_sym.has_value()); // Not a DFA.
                assert(!outgoing.test(t.maybe_sym.value())); // Not a DFA.
                outgoing.set(t.maybe_sym.value());
            }

            for (size_t sym = 0; sym < outgoing.size(); ++sym) {
                if (outgoing.test(sym))
                    continue;

                // Look up where this edge goes from the start state
                // careful though, was `src` might _be_ the start state
                // in this case, we should add an edge to itself.
                if (src == START) {
                    this->add_transition(src, src, sym, true);
                    continue;
                }

                // Try to add the transition to the state after the start state.
                // If no such transition exists, the dfa would end up in a reject state
                // after this symbol. Just ignore it if so.
                for (const auto t : this->states[START].transitions) {
                    assert(t.maybe_sym.has_value());
                    if (t.maybe_sym.value() == sym) {
                        this->add_transition(src, t.dst, sym, true);
                        break;
                    }
                }
            }
        }
    }

    void FiniteStateAutomaton::build_lexer(std::span<const Token> tokens) {
        for (const auto& token : tokens) {
            auto regex_start = this->add_state();
            this->add_epsilon_transition(START, regex_start);
            auto regex_end = token.regex->compile(*this, regex_start);
            this->states[regex_end].token = &token;
        }
    }
}
