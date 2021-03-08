#ifndef _PAREAS_LPG_LEXER_FSA_HPP
#define _PAREAS_LPG_LEXER_FSA_HPP

#include <vector>
#include <iosfwd>
#include <optional>
#include <cstddef>
#include <cstdint>

namespace pareas::lexer {
    struct Token;
    struct LexicalGrammar;

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
            std::optional<uint8_t> maybe_sym;
            StateIndex dst;
            bool produces_token;
        };

        struct State {
            const Token* token;
            std::vector<Transition> transitions;
        };

        std::vector<State> states;

        FiniteStateAutomaton();

        size_t num_states() const;

        StateIndex add_state();

        void add_transition(StateIndex src, StateIndex dst, std::optional<uint8_t> sym, bool produces_token = false);
        void add_epsilon_transition(StateIndex src, StateIndex dst);

        State& operator[](StateIndex state);
        const State& operator[](StateIndex state) const;

        void dump_dot(std::ostream& os) const;

        FiniteStateAutomaton to_dfa() const;

        void add_lexer_loop();

        void build_lexer(const LexicalGrammar* g);
    };
}

#endif
