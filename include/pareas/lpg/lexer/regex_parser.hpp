#ifndef _PAREAS_LPG_LEXER_REGEX_PARSER_HPP
#define _PAREAS_LPG_LEXER_REGEX_PARSER_HPP

#include "pareas/lpg/lexer/regex.hpp"
#include "pareas/lpg/parser.hpp"

#include <stdexcept>
#include <cstdint>

namespace pareas::lexer {
    struct RegexParseError: std::runtime_error {
        RegexParseError(): std::runtime_error("Parse error") {}
    };

    class RegexParser {
        Parser* parser;

    public:
        RegexParser(Parser* parser);
        UniqueRegexNode parse();

    private:
        UniqueRegexNode alternation();
        UniqueRegexNode sequence();
        UniqueRegexNode maybe_repeat();
        UniqueRegexNode maybe_atom();
        UniqueRegexNode group();
        uint8_t escaped_char();
    };
}

#endif
