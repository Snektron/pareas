#ifndef _PAREAS_LEXGEN_PARSER_HPP
#define _PAREAS_LEXGEN_PARSER_HPP

#include "pareas/common/parser.hpp"

#include "pareas/lexgen/regex.hpp"

#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <string_view>
#include <cstddef>

namespace pareas {
    struct LexerParseError: std::runtime_error {
        LexerParseError(): std::runtime_error("Parse error") {}
    };

    struct Token {
        size_t priority;
        SourceLocation loc;
        std::string_view name;
        UniqueRegexNode regex;
    };

    class LexerParser {
        Parser* parser;

        std::vector<Token> tokens;
        std::unordered_map<std::string_view, SourceLocation> token_definitions;

    public:
        LexerParser(Parser* parser);
        std::vector<Token>&& parse();

    private:
        bool token_decl();
    };
}

#endif
