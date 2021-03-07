#ifndef _PAREAS_LPG_LEXER_INTERPRETER_HPP
#define _PAREAS_LPG_LEXER_INTERPRETER_HPP

#include "pareas/lpg/lexer/parallel_lexer.hpp"

#include <string_view>

namespace pareas {
    struct LexerInterpreter {
        const ParallelLexer* lexer;

        LexerInterpreter(const ParallelLexer* lexer);

        void lex_linear(std::string_view input) const;
    };
}

#endif
