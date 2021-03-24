#include "pareas/lpg/lexer/lexer_parser.hpp"
#include "pareas/lpg/lexer/regex_parser.hpp"

#include <fmt/format.h>

#include <cassert>

namespace pareas::lexer {
    LexerParser::LexerParser(Parser* parser):
        parser(parser) {}

    LexicalGrammar LexerParser::parse() {
        bool error = false;
        this->parser->eat_delim();
        while (auto _ = this->parser->peek()) {
            if (!this->lexeme_decl()) {
                error = true;
                this->parser->skip_until('\n');
            }
            this->parser->eat_delim();
        }

        error |= !this->insert_precedes();

        if (error)
            throw LexerParseError();

        return {std::move(this->lexemes)};
    }

    bool LexerParser::lexeme_decl() {
        auto loc = this->parser->loc();
        auto lexeme_name = this->parser->word();

        if (lexeme_name.size() == 0)
            return false;

        this->parser->eat_delim(false);
        if (!this->parser->expect('='))
            return false;

        this->parser->eat_delim(false);

        auto regex_parser = RegexParser(this->parser);
        auto root = UniqueRegexNode(nullptr);
        try {
            root = regex_parser.parse();
        } catch (const RegexParseError&) {
            // Don't propagate error, as we will attempt to recover in the main loop
            // of the lexer parser
            return false;
        }

        this->parser->eat_delim(false);

        auto preceded_by = std::unordered_set<std::string_view>();
        if (this->parser->test('[') && !this->precede_list(preceded_by))
            return false;

        auto lexeme_index = this->lexemes.size();
        auto [it, inserted] = this->lexeme_definitions.insert({lexeme_name, {lexeme_index, std::move(preceded_by)}});
        if (!inserted) {
            const auto& previous = this->lexemes[it->second.index];
            this->parser->er->error(loc, "Duplicate lexeme definition");
            this->parser->er->note(previous.loc, "First defined here");
            return false;
        }

        this->lexemes.push_back({loc, std::string(lexeme_name), std::move(root)});

        return this->parser->expect('\n');
    }

    bool LexerParser::precede_list(std::unordered_set<std::string_view>& preceded_by) {
        if (!this->parser->expect('[')) {
            return false;
        }

        this->parser->eat_delim(false);

        if (this->parser->eat(']'))
            return true;

        do {
            this->parser->eat_delim(false);
            auto name = this->parser->word();
            if (name.size() == 0)
                return false;

            preceded_by.insert(name);

            this->parser->eat_delim(false);
        } while (this->parser->eat(','));

        return this->parser->expect(']');
    }

    bool LexerParser::insert_precedes() {
        bool error = false;

        for (const auto& [lexeme_name, definition] : this->lexeme_definitions) {
            auto& lexeme = this->lexemes[definition.index];

            for (const auto& prec_name : definition.preceded_by) {
                auto it = this->lexeme_definitions.find(prec_name);
                if (it == this->lexeme_definitions.end()) {
                    this->parser->er->error(lexeme.loc, fmt::format("Undefined lexeme '{}'", prec_name));
                    error = true;
                    continue;
                }

                const auto& prec = this->lexemes[it->second.index];
                lexeme.preceded_by.push_back(&prec);
            }
        }

        return !error;
    }
}
