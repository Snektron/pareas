#include "pareas/lexgen/parallel_lexer.hpp"
#include "pareas/lexgen/fsa.hpp"
#include "pareas/common/hash_util.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <unordered_map>
#include <queue>
#include <cassert>

namespace {
    using namespace pareas;
    using StateIndex = ParallelLexer::StateIndex;
    using Transition = ParallelLexer::Transition;

    struct ParallelState {
        std::vector<Transition> transitions;

        ParallelState(size_t states);
        void merge(const ParallelState& other);
    };

    ParallelState::ParallelState(size_t states):
        transitions(states) {
    }

    void ParallelState::merge(const ParallelState& other) {
        for (auto& state : this->transitions) {
            state = other.transitions[state.result_state];
        }
    }

    bool operator==(const ParallelState& lhs, const ParallelState& rhs) {
        return std::equal(
            lhs.transitions.begin(),
            lhs.transitions.end(),
            rhs.transitions.begin(),
            rhs.transitions.end(),
            [](const auto& lhs, const auto& rhs) {
                return lhs.result_state == rhs.result_state && lhs.produces_token == rhs.produces_token;
            }
        );
    }
}

template <>
struct std::hash<ParallelState> {
    size_t operator()(const ParallelState& ps) const {
        size_t hash = 0;
        for (auto state : ps.transitions) {
            hash = pareas::hash_combine(hash, state.result_state);
            hash = pareas::hash_combine(hash, state.produces_token);
        }
        return hash;
    }
};

namespace pareas {
    ParallelLexer::Transition::Transition():
        result_state(REJECT), produces_token(false) {}

    ParallelLexer::MergeTable::MergeTable():
        num_states(0), capacity(0), merge_table(nullptr) {}

    void ParallelLexer::MergeTable::resize(size_t new_num_states) {
        if (new_num_states <= this->capacity) {
            this->num_states = new_num_states;
            return;
        }

        // Crude computation of new capacity, but it'll do
        auto new_capacity = std::max(MIN_SIZE, this->capacity);
        while (new_capacity < new_num_states)
            new_capacity *= GROW_FACTOR;

        auto new_ptr = std::make_unique<Transition[]>(new_capacity * new_capacity);
        for (size_t second = 0; second < this->num_states; ++second) {
            for (size_t first = 0; first < this->num_states; ++first) {
                new_ptr[first + second * new_capacity] = (*this)(first, second);
            }
        }

        this->num_states = new_num_states;
        this->capacity = new_capacity;
        this->merge_table = std::move(new_ptr);
    }

    size_t ParallelLexer::MergeTable::index(StateIndex first, StateIndex second) const {
        assert(first <= this->num_states);
        assert(second <= this->num_states);
        return first + second * this->capacity;
    }

    auto ParallelLexer::MergeTable::operator()(StateIndex first, StateIndex second) -> Transition& {
        return this->merge_table[this->index(first, second)];
    }

    auto ParallelLexer::MergeTable::operator()(StateIndex first, StateIndex second) const -> const Transition& {
        return this->merge_table[this->index(first, second)];
    }

    size_t ParallelLexer::MergeTable::states() const {
        return this->num_states;
    }

    ParallelLexer::ParallelLexer(std::span<const Token> tokens) {
        auto max_sym = std::numeric_limits<FiniteStateAutomaton::Symbol>::max();
        auto num_syms = max_sym + 1;

        auto nfa = FiniteStateAutomaton();
        nfa.build_lexer(tokens);
        auto dfa = nfa.to_dfa();
        dfa.add_lexer_loop();

        auto seen = std::unordered_map<ParallelState, StateIndex>();
        auto states = std::vector<ParallelState>();
        auto transitions = std::vector<Transition>();

        auto enqueue = [&](ParallelState&& ps) {
            auto [it, inserted] = seen.insert({std::move(ps), states.size()});
            if (inserted) {
                states.push_back(it->first);
                this->merge_table.resize(it->second + 1);
            }
            return it->second;
        };

        // Insert the initial states, we need to insert one for every character.
        // States indices of the DFA are mapped to the initial parallel states indices.
        {
            auto initial_states = std::vector<ParallelState>(num_syms, ParallelState(dfa.num_states()));
            for (size_t src = 0; src < dfa.num_states(); ++src) {
                for (const auto [sym, dst, produces_token] : dfa[src].transitions) {
                    assert(sym.has_value()); // Not a DFA
                    initial_states[sym.value()].transitions[src].result_state = dst;
                    initial_states[sym.value()].transitions[src].produces_token = produces_token;
                }
            }

            this->initial_states.resize(initial_states.size());
            for (int sym = 0; sym < initial_states.size(); ++sym) {
                auto& state = initial_states[sym];
                this->initial_states[sym].produces_token = state.transitions[START].produces_token;
                this->initial_states[sym].result_state = enqueue(std::move(state));
            }
        }

        // Add the identity mapping, which is required for futhark's scan operation.
        {
            auto identity = ParallelState(dfa.num_states());
            for (size_t i = 0; i < identity.transitions.size(); ++i) {
                identity.transitions[i].result_state = i;
            }
            this->identity_state_index = enqueue(std::move(identity));
        }

        // Repeatedly perform the merges until no new merge is added
        for (StateIndex i = 0; i < states.size(); ++i) {
            auto first = states[i];
            for (StateIndex j = 0; j < states.size(); ++j) {
                auto second = states[j];
                auto copy = first;
                copy.merge(second);
                second.merge(first);

                {
                    bool ij_produces_token = copy.transitions[START].produces_token;
                    auto ij = enqueue(std::move(copy));
                    this->merge_table(i, j).result_state = ij;
                    this->merge_table(i, j).produces_token = ij_produces_token;
                }

                {
                    bool ji_produces_token = second.transitions[START].produces_token;
                    auto ji = enqueue(std::move(second));
                    this->merge_table(j, i).result_state = ji;
                    this->merge_table(j, i).produces_token = ji_produces_token;
                }
            }
        }

        // Compute the final state mapping
        this->final_states.resize(seen.size(), nullptr);
        for (const auto& [ps, i] : seen) {
            this->final_states[i] = dfa[ps.transitions[START].result_state].token;
        }
    }
};
