#ifndef _PAREAS_LPG_LEXER_LEXICAL_GRAMMAR_HPP
#define _PAREAS_LPG_LEXER_LEXICAL_GRAMMAR_HPP

#include "pareas/lpg/error_reporter.hpp"
#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/lexer/regex.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>

namespace pareas::lexer {
    struct LexemeMatchesEmptyError: std::runtime_error {
        LexemeMatchesEmptyError(): std::runtime_error("Lexeme matches the empty string") {}
    };

    struct Lexeme {
        SourceLocation loc;
        std::string name;
        UniqueRegexNode regex;
        std::vector<const Lexeme*> preceded_by; // Should be unique

        Token as_token() const;
    };

    struct LexicalGrammar {
        std::vector<Lexeme> lexemes;

        size_t lexeme_id(const Lexeme* lexeme) const;

        void add_tokens(TokenMapping& tm) const;

        void validate(ErrorReporter& er) const;
    };
}

#endif
