#include "pareas/common/parser.hpp"

#include <fmt/format.h>

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
                    static_cast<char>(c)
                ));
            } else {
                this->er->error(this->loc(), fmt::format(
                    "Unexpected character '{}', expected '{}'",
                    static_cast<char>(actual),
                    static_cast<char>(c)
                ));
            }
            return false;
        }
        return true;
    }
}
