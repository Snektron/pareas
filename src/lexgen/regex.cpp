#include "pareas/lexgen/regex.hpp"
#include "pareas/common/escape.hpp"

#include <fmt/ostream.h>

#include <bitset>
#include <limits>
#include <cassert>

namespace pareas {
    void SequenceNode::print(std::ostream& os) const {
        if (this->children.size() == 1) {
            this->children[0]->print(os);
        } else {
            fmt::print("(");
            for (const auto& child : this->children) {
                child->print(os);
            }
            fmt::print(")");
        }
    }

    auto SequenceNode::compile(FiniteStateAutomaton& fsa, StateIndex start) const -> StateIndex {
        StateIndex end = start;
        for (const auto& child : this->children) {
            end = child->compile(fsa, end);
        }
        return end;
    }

    void AlternationNode::print(std::ostream& os) const {
        if (this->children.size() == 1) {
            this->children[0]->print(os);
        } else {
            fmt::print("(");
            bool first = true;
            for (const auto& child : this->children) {
                if (first)
                    first = false;
                else
                    fmt::print("|");

                child->print(os);
            }
            fmt::print(")");
        }
    }

    auto AlternationNode::compile(FiniteStateAutomaton& fsa, StateIndex start) const -> StateIndex {
        if (this->children.empty())
            return start;

        auto end = fsa.add_state();
        for (const auto& child : this->children) {
            auto child_start = fsa.add_state();
            auto child_end = child->compile(fsa, child_start);
            fsa.add_epsilon_transition(start, child_start);
            fsa.add_epsilon_transition(child_end, end);
        }

        return end;
    }

    void RepeatNode::print(std::ostream& os) const {
        this->child->print(os);
        fmt::print(os, this->repeat_type == RepeatType::ZERO_OR_MORE ? "*" : "+");
    }

    auto RepeatNode::compile(FiniteStateAutomaton& fsa, StateIndex start) const -> StateIndex {
        auto loop_start = fsa.add_state();
        auto loop_end = this->child->compile(fsa, loop_start);
        auto end = fsa.add_state();

        if (this->repeat_type == RepeatType::ZERO_OR_MORE)
            fsa.add_epsilon_transition(start, end);

        fsa.add_epsilon_transition(start, loop_start);
        fsa.add_epsilon_transition(loop_end, end);
        fsa.add_epsilon_transition(loop_end, loop_start);

        return end;
    }

    void CharSetNode::print(std::ostream& os) const {
        fmt::print(os, "[{}", this->inverted ? "^" : "");

        for (const auto [min, max] : this->ranges) {
            if (min == max)
                fmt::print(os, "{:r}", EscapeFormatter{min});
            else
                fmt::print(os, "{:r}-{:r}", EscapeFormatter{min}, EscapeFormatter{max});
        }

        fmt::print(os, "]");
    }

    auto CharSetNode::compile(FiniteStateAutomaton& fsa, StateIndex start) const -> StateIndex {
        auto end = fsa.add_state();

        if (this->inverted) {
            auto bits = std::bitset<std::numeric_limits<unsigned char>::max()>();

            for (const auto [min, max] : this->ranges) {
                for (int c = min; c <= max; ++c) {
                    bits.set(c);
                }
            }

            for (int c = fsa.alphabet.min; c <= fsa.alphabet.max; ++c) {
                if (!bits.test(c))
                    fsa.add_transition(start, end, c);
            }
        } else {
            for (const auto [min, max] : this->ranges) {
                for (int c = min; c <= max; ++c) {
                    fsa.add_transition(start, end, c);
                }
            }
        }

        return end;
    }

    void CharNode::print(std::ostream& os) const {
        fmt::print(os, "{:r}", EscapeFormatter{this->c});
    }

    auto CharNode::compile(FiniteStateAutomaton& fsa, StateIndex start) const -> StateIndex {
        auto end = fsa.add_state();
        fsa.add_transition(start, end, this->c);
        return end;
    }

    void EmptyNode::print(std::ostream& is) const {}

    auto EmptyNode::compile(FiniteStateAutomaton& fsa, StateIndex start) const -> StateIndex {
        return start;
    }
}
