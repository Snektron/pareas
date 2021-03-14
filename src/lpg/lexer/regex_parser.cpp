#include "pareas/lpg/lexer/regex_parser.hpp"
#include "pareas/lpg/lexer/char_range.hpp"
#include "pareas/lpg/escape.hpp"

#include <fmt/format.h>

#include <cctype>
#include <cstdlib>

namespace {
    bool is_control_char(uint8_t c) {
        switch (c) {
            case '[':
            case ']':
            case '*':
            case '+':
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

namespace pareas::lexer {
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
        if (!this->parser->test('|'))
            return first;

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

        bool star = this->parser->eat('*');
        bool plus = this->parser->eat('+');

        if (!star && !plus)
            return child;

        if (!child) {
            this->parser->er->error(loc, "Stray repeat is not allowed to be applied to nothing");
            throw RegexParseError();
        }

        return std::make_unique<RepeatNode>(
            star ? RepeatType::ZERO_OR_MORE : RepeatType::ONE_OR_MORE,
            std::move(child)
        );
    }

    UniqueRegexNode RegexParser::maybe_atom() {
        auto c = this->parser->peek();
        auto loc = this->parser->loc();

        if (c == '[') {
            return this->group();
        } else if (this->parser->eat('(')) {
            auto child = this->alternation();
            if (!this->parser->expect(')'))
                throw RegexParseError();
            return child;
        } else if (c == '\\') {
            return std::make_unique<CharNode>(this->escaped_char());
        } else if (!c.has_value() || is_control_char(c.value())) {
            return nullptr; // nullptr used as optional here
        } else if (!std::isprint(c.value())) {
            this->parser->er->error(loc, fmt::format(
                "Unexpected character '{}', expected <printable character>",
                EscapeFormatter{c.value()}
            ));
            throw RegexParseError();
        }

        this->parser->consume();
        return std::make_unique<CharNode>(c.value());
    }

    UniqueRegexNode RegexParser::group() {
        auto parse_char = [&] {
            auto c = this->parser->peek();
            if (!c.has_value()) {
                this->parser->er->error(this->parser->loc(), "Unexpected EOF, expected <character>");
                throw RegexParseError();
            } else if (c == '\\') {
                return this->escaped_char();
            } else if (!std::isprint(c.value())) {
                this->parser->er->error(this->parser->loc(), fmt::format(
                    "Unexpected character '{}', expected <printable character>",
                    EscapeFormatter{c.value()}
                ));
                throw RegexParseError();
            }

            this->parser->consume();
            return c.value();
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
            uint8_t min = parse_char();
            if (!this->parser->eat('-')) {
                insert_range({min, min});
                continue;
            }

            uint8_t max = parse_char();
            if (min > max) {
                this->parser->er->error(this->parser->loc(), fmt::format(
                    "Invalid character range {} to {}: start code ({}) is greater than end code ({})",
                    EscapeFormatter{min},
                    EscapeFormatter{max},
                    min,
                    max
                ));
                throw RegexParseError();
            }

            insert_range({min, max});
        }

        return std::make_unique<CharSetNode>(std::move(ranges), inverted);
    }

    uint8_t RegexParser::escaped_char() {
        auto loc = this->parser->loc();

        auto next = [&]() {
            if (auto c = this->parser->consume())
                return c.value();
            this->parser->er->error(loc, "Unexpected EOF in escape sequence");
            throw RegexParseError();
        };

        auto convert_hex = [](int x) {
            if ('a' <= x && x <= 'f')
                return x - 'a';
            else if ('A' <= x && x <= 'Z')
                return x - 'A';
            else if ('0' <= x && x <= '9')
                return x - '0';
            return -1;
        };

        if (!this->parser->expect('\\'))
            throw RegexParseError();

        auto c = next();

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
                int hi = convert_hex(next());
                int lo = convert_hex(next());

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
