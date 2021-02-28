#ifndef _PAREAS_LEXGEN_PARALLEL_LEXER_HPP
#define _PAREAS_LEXGEN_PARALLEL_LEXER_HPP

#include "pareas/lexgen/lexer_parser.hpp"
#include "pareas/lexgen/fsa.hpp"

#include <span>
#include <memory>
#include <vector>
#include <limits>
#include <cstdint>

namespace pareas {
    struct ParallelLexer {
        using StateIndex = FiniteStateAutomaton::StateIndex;

        // The DFA states of the dfa corresponding to the lexer are mapped to the first
        // few entries of the state reduction table, and so the reject and start
        // states appear in the same offsets.
        constexpr const static StateIndex REJECT = FiniteStateAutomaton::REJECT;
        constexpr const static StateIndex START = FiniteStateAutomaton::START;

        struct Transition {
            StateIndex result_state;
            bool produces_token;

            Transition();
        };

        class MergeTable {
            constexpr const static size_t GROW_FACTOR = 2;
            constexpr const static size_t MIN_SIZE = 16;

            size_t num_states;
            size_t capacity;
            std::unique_ptr<Transition[]> merge_table;

        public:
            MergeTable();

            void resize(size_t num_states);

            size_t index(StateIndex first, StateIndex second) const;

            Transition& operator()(StateIndex first, StateIndex second);
            const Transition& operator()(StateIndex first, StateIndex second) const;
        };

        // Char to initial state
        // Moving from the initial state could also produce a transition,
        // if the start state is accepting.
        std::vector<StateIndex> initial_states;

        // TODO: State to new state table
        MergeTable merge_table;

        // ParallelState to token they might produce
        std::vector<const Token*> final_states;

        explicit ParallelLexer(std::span<const Token> tokens);
    };
}

#endif
