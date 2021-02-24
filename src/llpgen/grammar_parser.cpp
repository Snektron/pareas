#include "pareas/llpgen/grammar_parser.hpp"

#include <fmt/ostream.h>

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

namespace pareas {
    GrammarParser::GrammarParser(ErrorReporter* er, std::string_view source):
        er(er), source(source), offset(0),
        start{"", {0}}, left_delim{"", {0}}, right_delim{"", {0}} {}

    Grammar GrammarParser::parse() {
        bool error = false;

        this->eat_delim();
        int c;
        while ((c = this->peek()) != EOF) {
            bool ok = c == '%' ? this->directive() : this->production();
            if (!ok) {
                error = true;
                this->skip_statement();
            }
            this->eat_delim();
        }

        if (this->start.value.size() == 0) {
            this->er->error(this->loc(), "Missing directive %start");
            error = true;
        }

        if (this->left_delim.value.size() == 0) {
            this->er->error(this->loc(), "Missing directive %left_delim");
            error = true;
        }

        if (this->right_delim.value.size() == 0) {
            this->er->error(this->loc(), "Missing directive %right_delim");
            error = true;
        }

        const auto* start = this->find_start_rule();

        if (error || !start)
            throw GrammarParseError();

        auto g = Grammar{
            .left_delim = Terminal{std::string(this->left_delim.value)},
            .right_delim = Terminal{std::string(this->right_delim.value)},
            .start = start,
            .productions = std::move(this->productions),
        };
        g.validate(*this->er);
        return g;
    }

    SourceLocation GrammarParser::loc() const {
        return {this->offset};
    }

    const Production* GrammarParser::find_start_rule() const {
        // Find the start rule
        // Only one is allowed
        const Production* start = nullptr;
        bool error = false;

        auto start_nt = NonTerminal{std::string(this->start.value)};

        for (const auto& prod : this->productions) {
            if (prod.lhs != start_nt)
                continue;
            if (start) {
                this->er->error(prod.loc, "Duplicate start rule definition");
                this->er->note(start->loc, "First defined here");
                error = true;
            } else {
                start = &prod;
            }
        }

        if (!start) {
            this->er->error(this->loc(), "Missing start rule");
            return nullptr;
        }

        auto left_delim = Terminal{std::string(this->left_delim.value)};
        auto right_delim = Terminal{std::string(this->right_delim.value)};

        // Verify that the starting rule is of the right form
        if (start->rhs.empty() || start->rhs.front() != left_delim || start->rhs.back() != right_delim) {
            this->er->error(start->loc, "Start rule not in correct form");
            this->er->note(start->loc, fmt::format("Expected form {} -> '{}' ... '{}';", start->lhs, left_delim, right_delim));
            error = true;
        }

        if (error) {
            return nullptr;
        }

        return start;
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

    bool GrammarParser::expect(int c) {
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

    void GrammarParser::skip_statement() {
        while (true) {
            this->eat_delim(); // make sure to skip comments
            int c = this->consume();
            if (c == EOF || c == ';')
                break;
        }
    }

    bool GrammarParser::directive() {
        auto directive_loc = this->loc();
        if (!this->expect('%'))
            return false;
        auto name = this->word();
        if (name.size() == 0)
            return false;

        Directive* dir = nullptr;
        bool word = false;
        if (name == "start") {
            dir = &this->start;
            word = true;
        } else if (name == "left_delim") {
            dir = &this->left_delim;
        } else if (name == "right_delim") {
            dir = &this->right_delim;
        } else {
            this->er->error(directive_loc, fmt::format("Invalid directive '%{}'", name));
            return false;
        }

        bool error = false;
        if (dir->value.size() != 0) {
            this->er->error(directive_loc, fmt::format("Duplicate directive '%{}'", name));
            this->er->note(dir->loc, "First defined here");
            error = true;
        } else {
            dir->loc = directive_loc;
        }

        this->eat_delim();
        if (!this->expect('='))
            return false;
        this->eat_delim();

        auto value = word ? this->word() : this->terminal();
        if (value.size() == 0)
            return false;

        dir->value = value;
        this->eat_delim();
        return this->expect(';') && !error;
    }

    bool GrammarParser::production() {
        auto lhs_loc = this->loc();
        auto lhs = this->word();
        if (lhs.size() == 0)
            return false;

        this->eat_delim();

        auto tag_loc = lhs_loc;
        auto tag = lhs;
        if (this->peek() == '[') {
            tag_loc = this->loc();
            tag = this->tag();
            if (tag.size() == 0)
                return false;

            this->eat_delim();
        }

        if (!this->expect('-') || !this->expect('>'))
            return false;

        this->eat_delim();

        auto syms = std::vector<Symbol>();
        bool delimited = true;

        while (true) {
            int c = this->peek();
            auto sym_loc = this->loc();
            if (c == '\'') {
                auto t = this->terminal();
                if (t.size() == 0)
                    return false;
                syms.push_back(Terminal{std::string(t)});
            } else if (is_word_start_char(c)) {
                auto nt = this->word();
                if (nt.size() == 0)
                    return false;
                syms.push_back(NonTerminal{std::string(nt)});
            } else {
                break;
            }

            if (!delimited) {
                this->er->error(sym_loc, "Delimiter required between production RHS symbols");
                return false;
            }

            delimited = this->eat_delim();
        }

        if (!this->expect(';'))
            return false;

        auto it = this->tags.find(tag);
        if (it == this->tags.end()) {
            this->tags.insert(it, {tag, tag_loc});
        } else {
            this->er->error(tag_loc, fmt::format("Duplicate tag '{}'", tag));
            this->er->note(it->second, "First defined here");
            return false;
        }

        this->productions.push_back({lhs_loc, std::string(tag), NonTerminal{std::string(lhs)}, syms});
        return true;
    }

    std::string_view GrammarParser::word() {
        bool error = false;
        size_t start = this->offset;
        int c = this->peek();

        if (!is_word_start_char(c)) {
            this->er->error(this->loc(), fmt::format(
                "Invalid character '{}', expected <word>",
                static_cast<char>(c)
            ));
            error = true;
        }

        this->consume();

        c = this->peek();
        while (is_word_continue_char(c)) {
            this->consume();
            c = this->peek();
        }

        if (error)
            return "";

        return this->source.substr(start, this->offset - start);
    }

    std::string_view GrammarParser::terminal() {
        if (!this->expect('\''))
            return "";

        auto word = this->word();
        if (word.size() == 0)
            return "";

        if (!this->expect('\''))
            return "";

        return word;
    }

    std::string_view GrammarParser::tag() {
        if (!this->expect('['))
            return "";

        auto word = this->word();
        if (word.size() == 0)
            return "";

        if (!this->expect(']'))
            return "";

        return word;
    }
}
