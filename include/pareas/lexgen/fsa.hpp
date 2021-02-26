#ifndef _PAREAS_LEXGEN_FSA_HPP
#define _PAREAS_LEXGEN_FSA_HPP

#include "pareas/lexgen/char_range.hpp"

#include <unordered_set>
#include <string>
#include <vector>
#include <span>
#include <iosfwd>
#include <cstddef>

namespace pareas {
    struct Token;

    struct FiniteStateAutomaton {
        using Symbol = int;
        static constexpr const Symbol EPSILON = -1;

        using StateIndex = size_t;
        static constexpr const Symbol START = 0;

        struct Transition {
            Symbol sym;
            StateIndex dst;
        };

        struct State {
            const Token* token;
            std::vector<Transition> transitions;
        };

        std::vector<State> states;
        CharRange alphabet;

        FiniteStateAutomaton(CharRange alphabet);

        size_t num_states() const;

        StateIndex add_state();

        void add_transition(StateIndex src, StateIndex dst, Symbol sym);
        void add_epsilon_transition(StateIndex src, StateIndex dst);

        State& operator[](StateIndex state);
        const State& operator[](StateIndex state) const;

        void dump_dot(std::ostream& os) const;

        FiniteStateAutomaton to_dfa() const;

        void add_lexer_loop();

        static FiniteStateAutomaton build_lexer_nfa(CharRange alphabet, std::span<const Token> tokens);
    };
}

#endif
