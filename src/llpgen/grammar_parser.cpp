#include "pareas/llpgen/grammar_parser.hpp"

#include <fmt/ostream.h>

#include <algorithm>
#include <cstdio>

namespace pareas {
    GrammarParser::GrammarParser(Parser* parser):
        parser(parser) {}

    Grammar GrammarParser::parse() {
        bool error = false;

        this->parser->eat_delim();

        while (auto c = this->parser->peek()) {
            if (!this->production()) {
                error = true;
                this->parser->skip_until(';');
            }
            this->parser->eat_delim();
        }

        if (this->productions.size() > Grammar::START_INDEX) {
            auto* start = &this->productions[Grammar::START_INDEX];
            start->rhs.insert(start->rhs.begin(), Terminal::START_OF_INPUT);
            start->rhs.insert(start->rhs.end(), Terminal::END_OF_INPUT);
        } else {
            // Checked by g.validate()
        }

        if (error)
            throw GrammarParseError();

        auto g = Grammar{std::move(this->productions)};
        g.validate(*this->parser->er);
        return g;
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
                syms.push_back(Terminal{Terminal::Type::USER_DEFINED, std::string(t)});
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
