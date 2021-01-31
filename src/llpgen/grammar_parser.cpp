#include "pareas/llpgen/grammar_parser.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>

namespace {
    bool is_word_start_char(int c) {
        return std::isalpha(c) || c == '_';
    }

    bool is_word_continue_char(int c) {
        return std::isalnum(c) || c == '_';
    }
}

GrammarParser::GrammarParser(ErrorReporter* er, std::string_view source):
    er(er), source(source), offset(0) {}

Grammar GrammarParser::parse() {
    auto [start, left_delim, right_delim] = this->directives();
    auto productions = std::vector<Production>();
    if (!this->productions(productions))
        throw ParseError();

    return Grammar(
        NonTerminal{std::string(start)},
        Terminal{std::string(left_delim)},
        Terminal{std::string(right_delim)},
        std::move(productions)
    );
}

SourceLocation GrammarParser::loc() const {
    return {this->offset};
}

int GrammarParser::peek() {
    if (this->offset < this->source.size())
        return this->source[this->offset];
    return EOF;
}

int GrammarParser::consume() {
    int c = this->peek();
    if (c != EOF)
        ++this->offset;

    return c;
}

bool GrammarParser::eat(int c) {
    if (this->peek() == c) {
        this->consume();
        return true;
    }

    return false;
}

void GrammarParser::expect(int c) {
    if (!this->eat(c)) {
        int actual = this->peek();
        if (actual == EOF) {
            this->er->error_fmt(this->loc(), "Unexpected EOF, expected '", static_cast<char>(c), "'");
        } else {
            this->er->error_fmt(this->loc(), "Unexpected character '", static_cast<char>(actual), "', expected '", static_cast<char>(c), "'");
        }
        throw ParseError();
    }
}

bool GrammarParser::eat_delim() {
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
}

GrammarParser::Directives GrammarParser::directives() {
    auto parse_directive = [&](std::string_view name, bool word) {
        this->eat_delim();
        auto directive_loc = this->loc();
        this->expect('%');
        auto actual_name = this->word();

        if (actual_name != name) {
            this->er->error_fmt(directive_loc, "Unexpected directive %", actual_name, ", expected %", name);
            throw ParseError();
        }

        this->eat_delim();
        this->expect('=');
        this->eat_delim();
        auto value = word ? this->word() : this->terminal();
        this->eat_delim();
        this->expect(';');
        return value;
    };

    return Directives{
        .start = parse_directive("start", true),
        .left_delim = parse_directive("left_delim", false),
        .right_delim = parse_directive("right_delim", false),
    };
}

bool GrammarParser::productions(std::vector<Production>& productions) {
    bool all_good = true;

    this->eat_delim();
    while (this->peek() != EOF) {
        try {
            this->production(productions);
        } catch (const ParseError& e) {
            all_good = false;

            // Skip until after the next ; (or EOF)
            while (true) {
                this->eat_delim(); // make sure to skip comments
                int c = this->consume();
                if (c == EOF || c == ';')
                    break;
            }
        }
        this->eat_delim();
    }

    return all_good;
}

void GrammarParser::production(std::vector<Production>& productions) {
    auto lhs_loc = this->loc();
    auto lhs = this->word();
    this->eat_delim();

    if (lhs == "_") {
        this->er->error(lhs_loc, "Empty symbol cannot be LHS of production");
        throw ParseError();
    }

    auto tag_loc = lhs_loc;
    auto tag = lhs;
    if (this->peek() == '[') {
        tag_loc = this->loc();
        tag = this->tag();
        this->eat_delim();
    }

    this->expect('-');
    this->expect('>');
    this->eat_delim();

    auto syms = std::vector<Symbol>();
    bool delimited = true;

    while (true) {
        int c = this->peek();
        auto sym_loc = this->loc();
        if (c == '\'') {
            auto t = this->terminal();
            syms.push_back(Terminal{std::string(t)});
        } else if (is_word_start_char(c)) {
            auto nt = this->word();

            if (nt == "_")
                syms.push_back(Terminal::null());
            else
                syms.push_back(NonTerminal{std::string(nt)});
        } else {
            break;
        }

        if (!delimited) {
            this->er->error(sym_loc, "Delimiter required between production RHS symbols");
            throw ParseError();
        }

        delimited = this->eat_delim();
    }

    this->expect(';');

    auto it = this->tags.find(tag);
    if (it == this->tags.end()) {
        this->tags.insert(it, {tag, tag_loc});
    } else {
        this->er->error_fmt(tag_loc, "Duplicate tag '", tag, "'");
        this->er->note(it->second, "First defined here");
        throw ParseError();
    }

    productions.push_back({std::string(tag), NonTerminal{std::string(lhs)}, syms});
}

std::string_view GrammarParser::word() {
    size_t start = this->offset;
    int c = this->peek();

    if (!is_word_start_char(c)) {
        this->er->error_fmt(this->loc(), "Invalid character '", static_cast<char>(c), "', expected <word>");
        throw ParseError();
    }

    this->consume();

    c = this->peek();
    while (is_word_continue_char(c)) {
        this->consume();
        c = this->peek();
    }

    return this->source.substr(start, this->offset - start);
}

std::string_view GrammarParser::terminal() {
    this->expect('\'');
    auto word = this->word();
    this->expect('\'');
    return word;
}

std::string_view GrammarParser::tag() {
    this->expect('[');
    auto word = this->word();
    this->expect(']');
    return word;
}
