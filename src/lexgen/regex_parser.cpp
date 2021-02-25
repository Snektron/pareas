#include "pareas/lexgen/regex_parser.hpp"
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
            c = this->escaped_char();
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
        return nullptr;
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
