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

ParseError::ParseError(const std::string& message):
    InvalidGrammarError(message) {}

GrammarParser::GrammarParser(std::string_view source):
    source(source), offset(0), line(0), column(0) {}

Grammar GrammarParser::parse() {
    auto [start, left_delim, right_delim] = this->directives();
    auto g = Grammar(
        NonTerminal{std::string(start)},
        Terminal{std::string(left_delim)},
        Terminal{std::string(right_delim)}
    );
    this->productions(g);
    return g;
}

std::string_view GrammarParser::current_line() const {
    size_t start = this->source.rfind('\n', std::max(this->offset, size_t{1}) - 1);
    if (start == std::string_view::npos)
        start = 0;
    else
        ++start;

    size_t end = this->source.find('\n', this->offset);
    if (end == std::string_view::npos)
        end = this->source.size();

    return this->source.substr(start, end - start);
}

int GrammarParser::peek() {
    if (this->offset < this->source.size())
        return this->source[this->offset];
    return EOF;
}

int GrammarParser::consume() {
    int c = this->peek();
    if (c == '\n') {
        ++this->line;
        this->column = 0;
    } else {
        ++this->column;
    }

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
        throw ParseError("Unexpected character");
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
        this->expect('%');
        auto actual_name = this->word();
        if (actual_name != name)
            throw ParseError("Invalid directive");
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

void GrammarParser::productions(Grammar& g) {
    this->eat_delim();
    while (this->peek() != EOF) {
        this->production(g);
        this->eat_delim();
    }
}

void GrammarParser::production(Grammar& g) {
    auto lhs = this->word();
    this->eat_delim();

    if (lhs == "_")
        throw ParseError("Invalid production LHS");

    auto tag = lhs;
    if (this->peek() == '[') {
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
        if (c == '\'') {
            if (!delimited)
                throw ParseError("Expected delimiter between rule arm symbols");

            auto t = this->terminal();
            syms.push_back(Terminal{std::string(t)});
        } else if (is_word_start_char(c)) {
            if (!delimited)
                throw ParseError("Expected delimiter between rule arm symbols");

            auto nt = this->word();

            if (nt == "_")
                syms.push_back(Terminal::null());
            else
                syms.push_back(NonTerminal{std::string(nt)});
        } else {
            break;
        }

        delimited = this->eat_delim();
    }

    this->expect(';');

    bool inserted = this->tags.insert(tag).second;
    if (!inserted)
        throw ParseError("Duplicate tag");

    g.add_rule({std::string(tag), NonTerminal{std::string(lhs)}, syms});
}

std::string_view GrammarParser::word() {
    size_t start = this->offset;
    int c = this->peek();

    if (!is_word_start_char(c))
        throw ParseError("Unexpected character");

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
