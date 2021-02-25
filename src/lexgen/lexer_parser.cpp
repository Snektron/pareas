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
}
