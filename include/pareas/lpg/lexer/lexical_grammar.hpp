#ifndef _PAREAS_LPG_LEXER_LEXICAL_GRAMMAR_HPP
#define _PAREAS_LPG_LEXER_LEXICAL_GRAMMAR_HPP

#include "pareas/lpg/error_reporter.hpp"
#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/lexer/regex.hpp"

#include <string>
#include <vector>
#include <cstddef>

namespace pareas::lexer {
    struct Lexeme {
        SourceLocation loc;
        std::string name;
        UniqueRegexNode regex;

        Token as_token() const;
    };

    struct LexicalGrammar {
        std::vector<Lexeme> lexemes;

        size_t lexeme_id(const Lexeme* lexeme) const;

        void add_tokens(TokenMapping& tm) const;
    };
}

#endif
