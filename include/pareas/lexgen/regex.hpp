#ifndef _PAREAS_LEXGEN_REGEX_HPP
#define _PAREAS_LEXGEN_REGEX_HPP

#include "pareas/lexgen/fsa.hpp"

#include <memory>
#include <vector>
#include <utility>
#include <iosfwd>

namespace pareas {
    struct RegexNode {
        using StateIndex = FiniteStateAutomaton::StateIndex;

        virtual void print(std::ostream& os) const = 0;
        virtual StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const = 0;

        virtual ~RegexNode() = default;
    };

    using UniqueRegexNode = std::unique_ptr<RegexNode>;

    struct SequenceNode: public RegexNode {
        std::vector<UniqueRegexNode> children;

        SequenceNode(std::vector<UniqueRegexNode>&& children):
            children(std::move(children)) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
    };

    struct AlternationNode: public RegexNode {
        std::vector<UniqueRegexNode> children;

        AlternationNode(std::vector<UniqueRegexNode>&& children):
            children(std::move(children)) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
    };

    struct RepeatNode: public RegexNode {
        UniqueRegexNode child;

        RepeatNode(UniqueRegexNode&& child):
            child(std::move(child)) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
    };

    struct CharSetNode: public RegexNode {
        struct Range {
            char min;
            char max;

            bool intersects(const Range& other) const;
            void merge(const Range& other);
        };

        std::vector<Range> ranges;
        bool inverted;

        CharSetNode(std::vector<Range>&& ranges, bool inverted):
            ranges(std::move(ranges)), inverted(inverted) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
    };

    struct CharNode: public RegexNode {
        char c;

        CharNode(char c):
            c(c) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
    };

    struct EmptyNode: public RegexNode {
        EmptyNode() = default;

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
    };
}

#endif
