#ifndef _PAREAS_LPG_LEXER_PARALLEL_LEXER_HPP
#define _PAREAS_LPG_LEXER_PARALLEL_LEXER_HPP

#include "pareas/lpg/lexer/lexical_grammar.hpp"
#include "pareas/lpg/lexer/fsa.hpp"

#include <span>
#include <memory>
#include <vector>
#include <limits>
#include <iosfwd>
#include <cstdint>

namespace pareas::lexer {
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

            size_t states() const;
        };

        // Char to initial state
        // Moving from the initial state could also produce a transition,
        // if the start state is accepting.
        std::vector<Transition> initial_states;

        // TODO: State to new state table
        MergeTable merge_table;

        // ParallelState to token they might produce
        std::vector<const Token*> final_states;

        StateIndex identity_state_index;

        explicit ParallelLexer(const LexicalGrammar* g);

        void dump_sizes(std::ostream& out) const;
    };
}

#endif
