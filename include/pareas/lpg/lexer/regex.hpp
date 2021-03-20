#ifndef _PAREAS_LPG_LEXER_REGEX_HPP
#define _PAREAS_LPG_LEXER_REGEX_HPP

#include "pareas/lpg/lexer/fsa.hpp"
#include "pareas/lpg/lexer/char_range.hpp"

#include <memory>
#include <vector>
#include <utility>
#include <iosfwd>
#include <cstdint>

namespace pareas::lexer {
    struct RegexNode {
        using StateIndex = FiniteStateAutomaton::StateIndex;

        virtual void print(std::ostream& os) const = 0;
        virtual StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const = 0;
        virtual bool matches_empty() const = 0;

        virtual ~RegexNode() = default;
    };

    using UniqueRegexNode = std::unique_ptr<RegexNode>;

    struct SequenceNode: public RegexNode {
        std::vector<UniqueRegexNode> children;

        SequenceNode(std::vector<UniqueRegexNode>&& children):
            children(std::move(children)) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
        bool matches_empty() const override;
    };

    struct AlternationNode: public RegexNode {
        std::vector<UniqueRegexNode> children;

        AlternationNode(std::vector<UniqueRegexNode>&& children):
            children(std::move(children)) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
        bool matches_empty() const override;
    };

    enum class RepeatType {
        ZERO_OR_MORE,
        ONE_OR_MORE
    };

    struct RepeatNode: public RegexNode {
        RepeatType repeat_type;
        UniqueRegexNode child;

        RepeatNode(RepeatType repeat_type, UniqueRegexNode&& child):
            repeat_type(repeat_type), child(std::move(child)) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
        bool matches_empty() const override;
    };

    struct CharSetNode: public RegexNode {
        std::vector<CharRange> ranges;
        bool inverted;

        CharSetNode(std::vector<CharRange>&& ranges, bool inverted):
            ranges(std::move(ranges)), inverted(inverted) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
        bool matches_empty() const override;
    };

    struct CharNode: public RegexNode {
        uint8_t c;

        CharNode(uint8_t c):
            c(c) {}

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
        bool matches_empty() const override;
    };

    struct EmptyNode: public RegexNode {
        EmptyNode() = default;

        void print(std::ostream& os) const override;
        StateIndex compile(FiniteStateAutomaton& fsa, StateIndex start) const override;
        bool matches_empty() const override;
    };
}

#endif
