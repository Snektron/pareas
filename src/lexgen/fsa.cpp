#include "pareas/lexgen/fsa.hpp"
#include "pareas/common/escape.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cassert>

namespace pareas {
    auto FiniteStateAutomaton::add_state(bool accepting, const std::string& tag) -> StateIndex {
        StateIndex index = this->states.size();
        this->states.push_back({
            .accepting = accepting,
            .tag = tag,
            .transitions = {}
        });
        return index;
    }

    void FiniteStateAutomaton::add_transition(StateIndex src, StateIndex dst, Symbol sym) {
        assert(src < this->states.size());
        assert(dst < this->states.size());

        this->states[src].transitions.push_back({sym, dst});
    }

    void FiniteStateAutomaton::add_epsilon_transition(StateIndex src, StateIndex dst) {
        this->add_transition(src, dst, EPSILON);
    }

    auto FiniteStateAutomaton::operator[](StateIndex state) -> State& {
        assert(state < this->states.size());
        return this->states[state];
    }

    auto FiniteStateAutomaton::operator[](StateIndex state) const -> const State& {
        assert(state < this->states.size());
        return this->states[state];
    }

    void FiniteStateAutomaton::dump_dot(std::ostream &os) const {
        fmt::print(os, "digraph {{\n");

        for (size_t src = 0; src < this->states.size(); ++src) {
            const auto& [accepting, tag, transitions] = this->states[src];
            fmt::print(
                "    state{} [shape=\"{}\", label=\"{}\"];\n",
                src,
                accepting ? "doublecircle" : "circle",
                tag
            );

            for (const auto& [sym, dst] : transitions) {
                if (sym == EPSILON) {
                    fmt::print("    state{} -> state{} [label=\"Ɛ\"];\n", src, dst);
                } else {
                    fmt::print("    state{} -> state{} [label=\"{:q}\"];\n", src, dst, EscapeFormatter{sym});
                }
            }
        }

        fmt::print(os, "}}\n");
    }
}
