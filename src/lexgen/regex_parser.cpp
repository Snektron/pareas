#include "pareas/lexgen/regex_parser.hpp"
#include "pareas/lexgen/char_range.hpp"
#include "pareas/common/escape.hpp"

#include <fmt/format.h>

#include <cctype>
#include <cstdlib>

namespace {
    bool is_control_char(int c) {
        switch (c) {
            case '[':
            case ']':
            case '*':
            case '(':
            case ')':
            case '/':
            case '|':
                return true;
            default:
                return false;
        }
    }
}

namespace pareas {
    RegexParser::RegexParser(Parser* parser):
        parser(parser) {}

    UniqueRegexNode RegexParser::parse() {
        if (!this->parser->expect('/'))
            throw RegexParseError();
        auto regex = this->alternation();
        if (!this->parser->expect('/'))
            throw RegexParseError();

        return regex;
    }

    UniqueRegexNode RegexParser::alternation() {
        auto first = this->sequence();
        if (this->parser->peek() != '|') {
            return first;
        }

        auto children = std::vector<UniqueRegexNode>();
        children.push_back(std::move(first));

        while (this->parser->eat('|')) {
            children.push_back(this->sequence());
        }

        return std::make_unique<AlternationNode>(std::move(children));
    }

    UniqueRegexNode RegexParser::sequence() {
        // If the first repeat matches nothing, then return an emopty sequence.
        auto first = this->maybe_repeat();
        if (!first)
            return std::make_unique<EmptyNode>();

        auto children = std::vector<UniqueRegexNode>();
        children.push_back(std::move(first));

        // Repeat while matching something.
        while (auto child = this->maybe_repeat()) {
            children.push_back(std::move(child));
        }

        return std::make_unique<SequenceNode>(std::move(children));
    }

    UniqueRegexNode RegexParser::maybe_repeat() {
        auto child = this->maybe_atom();
        auto loc = this->parser->loc();
        if (!this->parser->eat('*'))
            return child;

        if (!child) {
            this->parser->er->error(loc, "Stray star is not allowed to be applied to nothing");
            throw RegexParseError();
        }

        return std::make_unique<RepeatNode>(std::move(child));
    }

    UniqueRegexNode RegexParser::maybe_atom() {
        int c = this->parser->peek();
        auto loc = this->parser->loc();

        if (this->parser->peek() == '[') {
            return this->group();
        } else if (this->parser->eat('(')) {
            auto child = this->alternation();
            if (!this->parser->expect(')'))
                throw RegexParseError();
            return child;
        } else if (c == '\\') {
            return std::make_unique<CharNode>(this->escaped_char());
        } else if (is_control_char(c) || c == EOF) {
            return nullptr; // nullptr used as optional here
        } else if (!std::isprint(static_cast<unsigned char>(c))) {
            this->parser->er->error(loc, fmt::format(
                "Unexpected character '{}', expected <printable character>",
                EscapeFormatter{c}
            ));
            throw RegexParseError();
        }

        this->parser->consume();
        return std::make_unique<CharNode>(c);
    }

    UniqueRegexNode RegexParser::group() {
        auto parse_char = [&] {
            int c = this->parser->peek();
            if (c == EOF) {
                this->parser->er->error(this->parser->loc(), "Unexpected EOF, expected <character>");
                throw RegexParseError();
            } else if (c == '\\') {
                return this->escaped_char();
            } else if (!std::isprint(c)) {
                this->parser->er->error(this->parser->loc(), fmt::format(
                    "Unexpected character '{}', expected <printable character>",
                    EscapeFormatter{c}
                ));
                throw RegexParseError();
            }

            this->parser->consume();
            return c;
        };

        if (!this->parser->expect('['))
            throw RegexParseError();

        bool inverted = false;
        if (this->parser->eat('^'))
            inverted = true;

        auto ranges = std::vector<CharRange>();

        auto insert_range = [&](const CharRange& range) {
            for (auto& existing_range : ranges) {
                if (existing_range.intersecting_or_adjacent(range)) {
                    existing_range.merge(range);
                    return;
                }
            }

            ranges.push_back(range);
        };

        while (!this->parser->eat(']')) {
            char min = parse_char();
            if (!this->parser->eat('-')) {
                insert_range({static_cast<unsigned char>(min), static_cast<unsigned char>(min)});
                continue;
            }

            char max = parse_char();
            if (min > max) {
                this->parser->er->error(this->parser->loc(), fmt::format(
                    "Invalid character range {} to {}: start code ({}) is greater than end code ({})",
                    EscapeFormatter{min},
                    EscapeFormatter{max},
                    static_cast<int>(min),
                    static_cast<int>(max)
                ));
                throw RegexParseError();
            }

            insert_range({static_cast<unsigned char>(min), static_cast<unsigned char>(max)});
        }

        return std::make_unique<CharSetNode>(std::move(ranges), inverted);
    }

    int RegexParser::escaped_char() {
        auto convert_hex = [](int x) {
            if ('a' <= x && x <= 'f')
                return x - 'a';
            else if ('A' <= x && x <= 'Z')
                return x - 'A';
            else if ('0' <= x && x <= '9')
                return x - '0';
            return -1;
        };

        auto loc = this->parser->loc();
        if (!this->parser->expect('\\'))
            throw RegexParseError();

        int c = this->parser->consume();

        if (is_control_char(c))
            return c;

        switch (c) {
            case 'n':
                return '\n';
            case 'r':
                return '\r';
            case 't':
                return '\t';
            case '\\':
            case '\'':
            case '"':
            case '-':
            case '^':
                return c;
            case 'x': {
                int hi = convert_hex(this->parser->consume());
                int lo = convert_hex(this->parser->consume());

                if (hi < 0 || lo < 0) {
                    this->parser->er->error(loc, "Invalid hex escape sequence");
                    throw RegexParseError();
                }

                return hi * 16 + lo;
            }
            default:
                this->parser->er->error(loc, "Invalid escape sequence");
                throw RegexParseError();
        }
    }
}
