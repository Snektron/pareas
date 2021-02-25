#include "pareas/common/parser.hpp"
#include "pareas/common/escape.hpp"

#include <fmt/format.h>

#include <cctype>
#include <cstdlib>

namespace pareas {
    Parser::Parser(ErrorReporter* er, std::string_view source):
        er(er), source(source), offset(0) {}

    SourceLocation Parser::loc() const {
        return {this->offset};
    }

    int Parser::peek() const {
        if (this->offset < this->source.size())
            return this->source[this->offset];
        return EOF;
    }

    int Parser::consume() {
        int c = this->peek();
        if (c != EOF)
            ++this->offset;

        return c;
    }

    bool Parser::eat(int c) {
        if (this->peek() == c) {
            this->consume();
            return true;
        }

        return false;
    }

    bool Parser::expect(int c) {
        if (!this->eat(c)) {
            int actual = this->peek();
            if (actual == EOF) {
                this->er->error(this->loc(), fmt::format(
                    "Unexpected EOF, expected '{}'",
                    EscapeFormatter{c}
                ));
            } else {
                this->er->error(this->loc(), fmt::format(
                    "Unexpected character '{}', expected '{}'",
                    EscapeFormatter{actual},
                    EscapeFormatter{c}
                ));
            }
            return false;
        }
        return true;
    }

    bool Parser::eat_delim() {
        // Eat any delimiter, such as whitespace and comments
        bool delimited = false;

        while (true) {
            int c = this->peek();
            switch (c) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    this->consume();
                    break;
                case '#':
                    while (this->peek() != '\n' && this->peek() != EOF)
                        this->consume();
                    break;
                default:
                    return delimited;
            }
            delimited = true;
        }

        return delimited;
    }

    std::string_view Parser::word() {
        bool error = false;
        size_t start = this->offset;
        int c = this->peek();

        if (!this->is_word_start_char(c)) {
            this->er->error(this->loc(), fmt::format(
                "Invalid character '{}', expected <word>",
                static_cast<char>(c)
            ));
            error = true;
        }

        this->consume();

        c = this->peek();
        while (this->is_word_continue_char(c)) {
            this->consume();
            c = this->peek();
        }

        if (error)
            return "";

        return this->source.substr(start, this->offset - start);
    }

    void Parser::skip_until(int end) {
        while (true) {
            this->eat_delim(); // make sure to skip comments
            int c = this->consume();
            if (c == EOF || c == end)
                break;
        }
    }

    bool Parser::is_word_start_char(int c) const {
        return std::isalpha(c) || c == '_';
    }

    bool Parser::is_word_continue_char(int c) const {
        return std::isalnum(c) || c == '_';
    }
}
