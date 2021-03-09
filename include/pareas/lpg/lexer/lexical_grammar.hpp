#ifndef _PAREAS_LPG_LEXER_LEXICAL_GRAMMAR_HPP
#define _PAREAS_LPG_LEXER_LEXICAL_GRAMMAR_HPP

#include "pareas/lpg/error_reporter.hpp"
#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/lexer/regex.hpp"

#include <string_view>
#include <vector>
#include <cstddef>

namespace pareas::lexer {
    struct Token {
        SourceLocation loc;
        std::string_view name;
        UniqueRegexNode regex;
    };

    struct LexicalGrammar {
        std::vector<Token> tokens;

        size_t token_id(const Token* token) const;
        TokenMapping build_token_mapping() const;
    };
}

#endif
