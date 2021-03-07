#ifndef _PAREAS_LLPGEN_GRAMMAR_PARSER_HPP
#define _PAREAS_LLPGEN_GRAMMAR_PARSER_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/common/parser.hpp"

#include <string_view>
#include <unordered_map>
#include <cstddef>

namespace pareas {
    struct GrammarParseError: InvalidGrammarError {
        GrammarParseError(): InvalidGrammarError("Parse error") {}
    };

    class GrammarParser {
        Parser* parser;

        std::vector<Production> productions;
        std::unordered_map<std::string_view, SourceLocation> tags;

    public:
        GrammarParser(Parser* parser);
        Grammar parse();

    private:
        bool production();
        std::string_view terminal(); // quoted word
        std::string_view tag(); // [word]
    };
}

#endif
