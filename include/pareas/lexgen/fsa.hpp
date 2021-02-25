#ifndef _PAREAS_LEXGEN_FSA_HPP
#define _PAREAS_LEXGEN_FSA_HPP

#include "pareas/lexgen/char_range.hpp"

#include <string>
#include <vector>
#include <iosfwd>
#include <cstddef>

namespace pareas {
    struct FiniteStateAutomaton {
        using Symbol = int;
        static constexpr const Symbol EPSILON = -1;

        using StateIndex = size_t;
        struct Transition {
            Symbol sym;
            StateIndex dst_state;
        };

        struct State {
            bool accepting;
            std::string tag;
            std::vector<Transition> transitions;
        };

        std::vector<State> states;
        CharRange alphabet;

        FiniteStateAutomaton(CharRange alphabet):
            alphabet(alphabet) {}

        StateIndex add_state(bool accepting = false, const std::string& tag = "");
        void add_transition(StateIndex src, StateIndex dst, Symbol sym);
        void add_epsilon_transition(StateIndex src, StateIndex dst);
        State& operator[](StateIndex state);
        const State& operator[](StateIndex state) const;
        void dump_dot(std::ostream& os) const;
    };
}

#endif
