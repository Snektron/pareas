#include "pareas/llpgen/grammar_parser.hpp"

#include <fmt/ostream.h>

#include <algorithm>
#include <cstdio>

namespace pareas {
    GrammarParser::GrammarParser(Parser* parser):
        parser(parser), left_delim{"", {0}}, right_delim{"", {0}} {}

    Grammar GrammarParser::parse() {
        bool error = false;

        this->parser->eat_delim();

        while (auto c = this->parser->peek()) {
            bool ok = c == '%' ? this->directive() : this->production();
            if (!ok) {
                error = true;
                this->parser->skip_until(';');
            }
            this->parser->eat_delim();
        }

        if (this->left_delim.value.size() == 0) {
            this->parser->er->error(this->parser->loc(), "Missing directive %left_delim");
            error = true;
        }

        if (this->right_delim.value.size() == 0) {
            this->parser->er->error(this->parser->loc(), "Missing directive %right_delim");
            error = true;
        }

        if (error || !this->check_start_rule())
            throw GrammarParseError();

        auto g = Grammar{
            .left_delim = Terminal{std::string(this->left_delim.value)},
            .right_delim = Terminal{std::string(this->right_delim.value)},
            .productions = std::move(this->productions),
        };
        g.validate(*this->parser->er);
        return g;
    }

    bool GrammarParser::check_start_rule() const {
        // Only one start rule is allowed, and exactly one must exist
        if (this->productions.size() <= Grammar::START_INDEX) {
            this->parser->er->error(this->parser->loc(), "Missing start rule");
            return false;
        }

        auto* start = &this->productions[Grammar::START_INDEX];
        bool error = false;

        for (const auto& prod : this->productions) {
            if (&prod == start)
                continue;

            if (prod.lhs == start->lhs) {
                this->parser->er->error(prod.loc, "Duplicate start rule definition");
                this->parser->er->note(start->loc, "First defined here");
                error = true;
            }
        }

        auto left_delim = Terminal{std::string(this->left_delim.value)};
        auto right_delim = Terminal{std::string(this->right_delim.value)};

        // Verify that the starting rule is of the right form
        if (start->rhs.empty() || start->rhs.front() != left_delim || start->rhs.back() != right_delim) {
            this->parser->er->error(start->loc, "Start rule not in correct form");
            this->parser->er->note(start->loc, fmt::format("Expected form {} -> '{}' ... '{}';", start->lhs, left_delim, right_delim));
            error = true;
        }

        return !error;
    }

    bool GrammarParser::directive() {
        auto directive_loc = this->parser->loc();
        if (!this->parser->expect('%'))
            return false;
        auto name = this->parser->word();
        if (name.size() == 0)
            return false;

        Directive* dir = nullptr;
         if (name == "left_delim") {
            dir = &this->left_delim;
        } else if (name == "right_delim") {
            dir = &this->right_delim;
        } else {
            this->parser->er->error(directive_loc, fmt::format("Invalid directive '%{}'", name));
            return false;
        }

        bool error = false;
        if (dir->value.size() != 0) {
            this->parser->er->error(directive_loc, fmt::format("Duplicate directive '%{}'", name));
            this->parser->er->note(dir->loc, "First defined here");
            error = true;
        } else {
            dir->loc = directive_loc;
        }

        this->parser->eat_delim();
        if (!this->parser->expect('='))
            return false;
        this->parser->eat_delim();

        auto value = this->terminal();
        if (value.size() == 0)
            return false;

        dir->value = value;
        this->parser->eat_delim();
        return this->parser->expect(';') && !error;
    }

    bool GrammarParser::production() {
        auto lhs_loc = this->parser->loc();
        auto lhs = this->parser->word();
        if (lhs.size() == 0)
            return false;

        this->parser->eat_delim();

        auto tag_loc = lhs_loc;
        auto tag = lhs;
        if (this->parser->test('[')) {
            tag_loc = this->parser->loc();
            tag = this->tag();
            if (tag.size() == 0)
                return false;

            this->parser->eat_delim();
        }

        if (!this->parser->expect('-') || !this->parser->expect('>'))
            return false;

        this->parser->eat_delim();

        auto syms = std::vector<Symbol>();
        bool delimited = true;

        while (auto c = this->parser->peek()) {
            auto sym_loc = this->parser->loc();
            if (c == '\'') {
                auto t = this->terminal();
                if (t.size() == 0)
                    return false;
                syms.push_back(Terminal{std::string(t)});
            } else if (this->parser->is_word_start_char(c.value())) {
                auto nt = this->parser->word();
                if (nt.size() == 0)
                    return false;
                syms.push_back(NonTerminal{std::string(nt)});
            } else {
                break;
            }

            if (!delimited) {
                this->parser->er->error(sym_loc, "Delimiter required between production RHS symbols");
                return false;
            }

            delimited = this->parser->eat_delim();
        }

        if (!this->parser->expect(';'))
            return false;

        auto it = this->tags.find(tag);
        if (it == this->tags.end()) {
            this->tags.insert(it, {tag, tag_loc});
        } else {
            this->parser->er->error(tag_loc, fmt::format("Duplicate tag '{}'", tag));
            this->parser->er->note(it->second, "First defined here");
            return false;
        }

        this->productions.push_back({lhs_loc, std::string(tag), NonTerminal{std::string(lhs)}, syms});
        return true;
    }

    std::string_view GrammarParser::terminal() {
        if (!this->parser->expect('\''))
            return "";

        auto word = this->parser->word();
        if (word.size() == 0)
            return "";

        if (!this->parser->expect('\''))
            return "";

        return word;
    }

    std::string_view GrammarParser::tag() {
        if (!this->parser->expect('['))
            return "";

        auto word = this->parser->word();
        if (word.size() == 0)
            return "";

        if (!this->parser->expect(']'))
            return "";

        return word;
    }
}
