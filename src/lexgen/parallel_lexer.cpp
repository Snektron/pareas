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
    ParallelLexer::Transition::Transition():
        Transition(false, REJECT) {}

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

    size_t ParallelLexer::MergeTable::index(ParallelStateIndex first, ParallelStateIndex second) const {
        assert(first <= this->num_states);
        assert(second <= this->num_states);
        return first + second * this->capacity;
    }

    auto ParallelLexer::MergeTable::operator()(ParallelStateIndex first, ParallelStateIndex second) -> Transition& {
        return this->merge_table[this->index(first, second)];
    }

    auto ParallelLexer::MergeTable::operator()(ParallelStateIndex first, ParallelStateIndex second) const -> const Transition& {
        return this->merge_table[this->index(first, second)];
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
            auto [it, inserted] = seen.insert({std::move(ps), states.size()});
            if (inserted) {
                states.push_back(it->first);
                this->merge_table.resize(it->second);
            }
            assert(it->second < Transition::PRODUCES_TOKEN_MASK);
            return it->second;
        };

        // Insert the initial states, we need to insert one for every character.
        // States indices of the DFA are mapped to the initial parallel states indices.
        {
            auto initial_states = std::vector<ParallelState>(num_syms, ParallelState(dfa.num_states()));
            for (size_t src = 0; src < dfa.num_states(); ++src) {
                for (const auto [sym, dst, poduces_token] : dfa[src].transitions) {
                    assert(sym.has_value()); // Not a DFA
                    initial_states[sym.value()].transitions[src] = dst;
                }
            }

            this->initial_states.resize(initial_states.size());
            for (int sym = 0; sym < initial_states.size(); ++sym) {
                this->initial_states[sym] = enqueue(std::move(initial_states[sym]));
            }
        }

        // Repeatedly perform the merges until no new merge is added
        for (ParallelStateIndex i = 0; i < states.size(); ++i) {
            auto first = states[i];
            for (ParallelStateIndex j = 0; j < states.size(); ++j) {
                auto second = states[j];
                auto copy = first;
                copy.merge(second);
                second.merge(first);

                enqueue(std::move(copy));
                enqueue(std::move(second));
            }
        }

        this->final_states.resize(seen.size(), nullptr);
        for (const auto& [ps, i] : seen) {
            this->final_states[i] = dfa[ps.transitions[START]].token;
        }
    }
};
