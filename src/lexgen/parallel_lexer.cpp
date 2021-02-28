#include "pareas/lexgen/parallel_lexer.hpp"
#include "pareas/lexgen/fsa.hpp"
#include "pareas/common/hash_util.hpp"

#include <fmt/format.h>

#include <queue>
#include <cassert>

namespace {
    using namespace pareas;

    struct ParallelState {
        std::vector<ParallelStateIndex> transitions;

        ParallelState(size_t states);
        void merge(const ParallelState& other);
    };

    ParallelState::ParallelState(size_t states):
        transitions(states, ParallelLexer::REJECT) {
    }

    void ParallelState::merge(const ParallelState& other) {
        for (auto& state : this->transitions) {
            state = other.transitions[state];
        }
    }

    bool operator==(const ParallelState& lhs, const ParallelState& rhs) {
        return std::equal(
            lhs.transitions.begin(),
            lhs.transitions.end(),
            rhs.transitions.begin(),
            rhs.transitions.end()
        );
    }
}

template <>
struct std::hash<ParallelState> {
    size_t operator()(const ParallelState& ps) const {
        size_t hash = 0;
        for (auto state : ps.transitions)
            hash = pareas::hash_combine(hash, state);
        return hash;
    }
};

namespace pareas {
    ParallelLexer::Transition::Transition(bool produces_token, ParallelStateIndex result_state):
        combined(produces_token ? result_state | PRODUCES_TOKEN_MASK : result_state) {
        assert(result_state < PRODUCES_TOKEN_MASK);
    }

    bool ParallelLexer::Transition::produces_token() const {
        return (this->combined & PRODUCES_TOKEN_MASK) != 0;
    }

    auto ParallelLexer::Transition::result_state() const -> ParallelStateIndex {
        return this->combined & ~PRODUCES_TOKEN_MASK;
    }

    ParallelLexer::ParallelLexer(std::span<const Token> tokens) {
        auto max_sym = std::numeric_limits<FiniteStateAutomaton::Symbol>::max();
        auto num_syms = max_sym + 1;

        auto nfa = FiniteStateAutomaton();
        nfa.build_lexer(tokens);
        auto dfa = nfa.to_dfa();
        dfa.add_lexer_loop();

        auto seen = std::unordered_map<ParallelState, ParallelStateIndex>();
        auto states = std::vector<ParallelState>();
        auto transitions = std::vector<Transition>();

        fmt::print("{} states\n", dfa.num_states());

        auto enqueue = [&](ParallelState&& ps) {
            auto [it, inserted] = seen.insert({std::move(ps), seen.size()});
            if (inserted) {
                states.push_back(it->first);
            }
            assert(it->second < Transition::PRODUCES_TOKEN_MASK);
            return it->second;
        };

        // Insert the initial states, we need to insert one for every character.
        // States indices of the DFA are mapped to the initial parallel states indices.
        {
            auto initial_states = std::vector<ParallelState>(num_syms, ParallelState(dfa.num_states()));
            for (size_t src = FiniteStateAutomaton::START; src < dfa.num_states(); ++src) {
                for (const auto [sym, dst, poduces_token] : dfa[src].transitions) {
                    assert(sym != FiniteStateAutomaton::Transition::EPSILON); // Not a DFA
                    initial_states[sym].transitions[src] = dst;
                }
            }

            this->initial_states.resize(initial_states.size());
            for (int sym = 0; sym < initial_states.size(); ++sym) {
                this->initial_states[sym] = enqueue(std::move(initial_states[sym]));
            }
        }

        // Repeatedly perform the merges until no new merge is added
        for (size_t i = 0; i < states.size(); ++i) {
            auto first = states[i];
            for (size_t j = 0; j < states.size(); ++j) {
                auto second = states[j];
                auto copy = first;
                copy.merge(second);
                second.merge(first);

                enqueue(std::move(copy));
                enqueue(std::move(second));
            }
        }

        fmt::print("{} states\n", states.size());

        this->final_states.resize(seen.size(), nullptr);
        for (const auto& [ps, i] : seen) {
            this->final_states[i] = dfa[ps.transitions[START]].token;
        }
    }
};
