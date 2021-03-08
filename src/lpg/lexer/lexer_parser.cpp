#include "pareas/lpg/lexer/lexer_parser.hpp"
#include "pareas/lpg/lexer/regex_parser.hpp"

#include <fmt/format.h>

namespace pareas::lexer {
    LexerParser::LexerParser(Parser* parser):
        parser(parser) {}

    LexicalGrammar LexerParser::parse() {
        bool error = false;
        this->parser->eat_delim();
        while (auto _ = this->parser->peek()) {
            if (!this->token_decl()) {
                error = true;
                this->parser->skip_until('\n');
            }
            this->parser->eat_delim();
        }

        if (error)
            throw LexerParseError();

        return {std::move(this->tokens)};
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

        // Only return after parsing the regex, so we also catch regex syntax errors
        if (duplicate)
            return false;

        this->tokens.push_back({loc, token_name, std::move(root)});

        return this->parser->expect('\n');
    }
}
