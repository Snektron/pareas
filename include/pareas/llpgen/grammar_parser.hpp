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
        struct Directive {
            std::string_view value;
            SourceLocation loc;
        };

        Parser* parser;

        std::vector<Production> productions;
        std::unordered_map<std::string_view, SourceLocation> tags;
        Directive left_delim, right_delim;

    public:
        GrammarParser(Parser* parser);
        Grammar parse();

    private:
        bool check_start_rule() const;

        bool directive();
        bool production();

        std::string_view terminal(); // quoted word
        std::string_view tag(); // [word]
    };
}

#endif
