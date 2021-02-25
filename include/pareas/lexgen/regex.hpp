#ifndef _PAREAS_LEXGEN_REGEX_HPP
#define _PAREAS_LEXGEN_REGEX_HPP

#include <memory>
#include <vector>
#include <utility>
#include <iosfwd>

namespace pareas {
    struct RegexNode {
        virtual void print(std::ostream& os) const = 0;

        virtual ~RegexNode() = default;
    };

    using UniqueRegexNode = std::unique_ptr<RegexNode>;

    struct SequenceNode: public RegexNode {
        std::vector<UniqueRegexNode> children;

        SequenceNode(std::vector<UniqueRegexNode>&& children):
            children(std::move(children)) {}

        void print(std::ostream& os) const override;
    };

    struct AlternationNode: public RegexNode {
        std::vector<UniqueRegexNode> children;

        AlternationNode(std::vector<UniqueRegexNode>&& children):
            children(std::move(children)) {}

        void print(std::ostream& os) const override;
    };

    struct RepeatNode: public RegexNode {
        UniqueRegexNode child;

        RepeatNode(UniqueRegexNode&& child):
            child(std::move(child)) {}

        void print(std::ostream& os) const override;
    };

    struct CharSetNode: public RegexNode {
        struct CharRange {
            char min;
            char max;
        };

        std::vector<CharRange> ranges;
        bool inverted;

        CharSetNode(std::vector<CharRange>&& ranges, bool inverted):
            ranges(std::move(ranges)), inverted(inverted) {}

        void print(std::ostream& os) const override;
    };

    struct CharNode: public RegexNode {
        char c;

        CharNode(char c):
            c(c) {}

        void print(std::ostream& os) const override;
    };

    struct EmptyNode: public RegexNode {
        EmptyNode() = default;

        void print(std::ostream& os) const override;
    };
}

#endif
