#ifndef _PAREAS_LPG_LEXER_LEXER_PARSER_HPP
#define _PAREAS_LPG_LEXER_LEXER_PARSER_HPP

#include "pareas/lpg/error_reporter.hpp"
#include "pareas/lpg/parser.hpp"
#include "pareas/lpg/lexer/lexical_grammar.hpp"
#include "pareas/lpg/lexer/regex.hpp"

#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <string_view>
#include <cstddef>

namespace pareas::lexer {
    struct LexerParseError: std::runtime_error {
        LexerParseError(): std::runtime_error("Parse error") {}
    };

    class LexerParser {
        Parser* parser;

        std::vector<Lexeme> lexemes;
        std::unordered_map<std::string_view, SourceLocation> lexeme_definitions;

    public:
        LexerParser(Parser* parser);
        LexicalGrammar parse();

    private:
        bool lexeme_decl();
    };
}

#endif
