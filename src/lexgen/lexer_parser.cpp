#include "pareas/lexgen/lexer_parser.hpp"
#include "pareas/lexgen/regex_parser.hpp"

#include <fmt/format.h>

namespace pareas {
    LexerParser::LexerParser(Parser* parser):
        parser(parser) {}

    std::vector<Token>&& LexerParser::parse() {
        bool error = false;
        this->parser->eat_delim();
        while (this->parser->peek() != EOF) {
            if (!this->token_decl()) {
                error = true;
                this->parser->skip_until('\n');
            }
            this->parser->eat_delim();
        }

        if (error)
            throw LexerParseError();

        return std::move(this->tokens);
    }

    void LexerParser::eat_whitespace() {
        // Eat whitespace that doesn't include newlines.
        while (true) {
            int c = this->parser->peek();
            switch (c) {
                case ' ':
                case '\t':
                case '\r':
                    this->parser->consume();
                default:
                    return;
            }
        }
    }

    bool LexerParser::token_decl() {
        auto loc = this->parser->loc();
        auto token_name = this->parser->word();

        if (token_name.size() == 0)
            return false;

        auto [it, inserted] = this->token_definitions.insert({token_name, loc});
        bool duplicate = false;
        if (!inserted) {
            this->parser->er->error(loc, "Duplicate token definition");
            this->parser->er->note(it->second, "First defined here");
            duplicate = true;
        }

        this->eat_whitespace();
        if (!this->parser->expect('='))
            return false;

        this->eat_whitespace();

        auto regex_parser = RegexParser(this->parser);
        auto root = UniqueRegexNode(nullptr);
        try {
            root = regex_parser.parse();
        } catch (const RegexParseError&) {
            // Don't propagate error, as we will attempt to recover in the main loop
            // of the lexer parser
            return false;
        }

        this->eat_whitespace();

        // Only return after parsing the regex, so we also catch regex syntax errors
        if (duplicate)
            return false;

        this->tokens.push_back({loc, token_name, std::move(root)});

        // A bit of a hack
        // TODO: Improve this
        if (this->parser->peek() == '#') {
            this->parser->eat_delim();
            return true;
        }

        return this->parser->expect('\n');
    }

    /*

    UniqueRegexNode LexerParser::regex() {
        return this->alternate();
    }

    UniqueRegexNode LexerParser::alternate() {
        auto first = this->sequence();

        if (!first || this->parser->peek() != '|')
            return first;

        auto children = std::vector<UniqueRegexNode>();
        children.push_back(std::move(first));

        while (this->parser->eat('|')) {
            auto child = this->sequence();
            if (!child)
                return child;

            children.push_back(std::move(child));
        }

        return std::make_unique<AlternationNode>(std::move(children));
    }

    UniqueRegexNode LexerParser::sequence() {
        // Closing things.
        // Chars that atoms may start with should not end a sequence!
        auto is_seq_end_char = [](int c) {
            return c == '|' || c == '/' || c == ')';
        };

        auto first = this->repeat();
        if (!first || is_seq_end_char(this->parser->peek())) {
            return first;
        }

        auto children = std::vector<UniqueRegexNode>();
        children.push_back(std::move(first));

        while (!is_seq_end_char(this->parser->peek())) {
            auto child = this->repeat();
            if (!child)
                return child;

            children.push_back(std::move(child));
        }

        return std::make_unique<SequenceNode>(std::move(children));    }

    UniqueRegexNode LexerParser::repeat() {
        auto child = this->atom();
        if (child && this->parser->eat('*'))
            return std::make_unique<RepeatNode>(std::move(child));
        return child;
    }

    UniqueRegexNode LexerParser::atom() {
        int c = this->parser->peek();

        if (c == '[') {
            return this->group();
        } else if (this->parser->eat('(')) {
            auto child = this->regex();
            if (!child || !this->parser->expect(')'))
                return nullptr;
            return child;
        }

        auto loc = this->parser->loc();

        if (c == '\\' && (c = this->escaped_char()) == EOF) {
            return nullptr;
        } else if (is_control_char(c)) {
            return std::make_unique<EmptyNode>();
        } else if (c == EOF || !std::isprint(static_cast<unsigned char>(c))) {
            this->parser->er->error(loc, "Unexpected character '{}', expected <printable character>");
            return nullptr;
        }

        this->parser->consume();
        return std::make_unique<CharNode>(c);
    }

    UniqueRegexNode LexerParser::group() {
        return nullptr;
    }

    int LexerParser::escaped_char() {
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
            return EOF;

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
                    return EOF;
                }

                return hi * 16 + lo;
            }
            default:
                this->parser->er->error(loc, "Invalid escape sequence");
                return EOF;
        }
    }

    */
}
