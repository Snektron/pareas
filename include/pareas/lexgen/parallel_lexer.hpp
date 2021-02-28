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
    // The most significant bit of this type is used to store whether the
    // transition produced a token in the merge table, so the upper bit may not
    // be used.
    using ParallelStateIndex = uint16_t;

    struct ParallelLexer {
        // The DFA states of the dfa corresponding to the lexer are mapped to the first
        // few entries of the state reduction table, and so the reject and start
        // states appear in the same offsets.
        constexpr const static ParallelStateIndex REJECT = FiniteStateAutomaton::REJECT;
        constexpr const static ParallelStateIndex START = FiniteStateAutomaton::START;

        struct Transition {
            constexpr const static ParallelStateIndex PRODUCES_TOKEN_MASK =
                1 << (std::numeric_limits<ParallelStateIndex>::digits - 1);

            ParallelStateIndex combined;

            Transition();
            Transition(bool produces_token, ParallelStateIndex result_state);
            bool produces_token() const;
            ParallelStateIndex result_state() const;
        };

        class MergeTable {
            constexpr const static size_t GROW_FACTOR = 2;
            constexpr const static size_t MIN_SIZE = 16;

            size_t num_states;
            size_t capacity;
            std::unique_ptr<Transition[]> merge_table;

        public:
            MergeTable();

            MergeTable(MergeTable&&) = delete;
            MergeTable& operator=(MergeTable&&) = delete;

            MergeTable(const MergeTable&) = delete;
            MergeTable& operator=(const MergeTable&) = delete;

            void resize(size_t num_states);

            size_t index(ParallelStateIndex first, ParallelStateIndex second) const;

            Transition& operator()(ParallelStateIndex first, ParallelStateIndex second);
            const Transition& operator()(ParallelStateIndex first, ParallelStateIndex second) const;
        };

        // Char to initial state
        // Moving from the initial state could also produce a transition,
        // if the start state is accepting.
        std::vector<ParallelStateIndex> initial_states;

        // TODO: State to new state table
        MergeTable merge_table;
        // std::unordered_Map<ParallelStateIndexPair, Transition> merge_table;

        // ParallelState to token they might produce
        std::vector<const Token*> final_states;

        explicit ParallelLexer(std::span<const Token> tokens);
    };
}

#endif
