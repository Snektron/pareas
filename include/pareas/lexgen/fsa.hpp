#ifndef _PAREAS_LEXGEN_FSA_HPP
#define _PAREAS_LEXGEN_FSA_HPP

#include <unordered_set>
#include <string>
#include <vector>
#include <span>
#include <iosfwd>
#include <limits>
#include <cstddef>
#include <cstdint>

namespace pareas {
    struct Token;

    struct FiniteStateAutomaton {
        using Symbol = uint8_t;
        using StateIndex = size_t;

        // An explicit reject state is added here to aid mapping FSA states to
        // parallel states during construction of the parallel lexer.
        // This FSA does not explicitly use reject states otherwise (missing transitions
        // are treated as rejects), but it makes no difference for the algoritms if they
        // are used either.
        static constexpr const StateIndex REJECT = 0;
        static constexpr const StateIndex START = 1;

        struct Transition {
            static constexpr const int EPSILON = -1;

            int maybe_sym;
            StateIndex dst;
            bool produces_token;

            bool is_epsilon_transition() const;
            uint8_t symbol() const;
        };

        struct State {
            const Token* token;
            std::vector<Transition> transitions;
        };

        std::vector<State> states;

        FiniteStateAutomaton();

        size_t num_states() const;

        StateIndex add_state();

        void add_transition(StateIndex src, StateIndex dst, uint8_t sym, bool produces_token = false);
        void add_epsilon_transition(StateIndex src, StateIndex dst);

        State& operator[](StateIndex state);
        const State& operator[](StateIndex state) const;

        void dump_dot(std::ostream& os) const;

        FiniteStateAutomaton to_dfa() const;

        void add_lexer_loop();

        void build_lexer(std::span<const Token> tokens);
    };
}

#endif
