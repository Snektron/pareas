#ifndef _PAREAS_LLPGEN_GRAMMAR_PARSER_HPP
#define _PAREAS_LLPGEN_GRAMMAR_PARSER_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/error_reporter.hpp"

#include <string_view>
#include <optional>
#include <unordered_map>
#include <cstddef>

struct ParseError: InvalidGrammarError {
    ParseError(): InvalidGrammarError("Parse error") {}
};

struct GrammarParser {
    ErrorReporter* er;
    std::string_view source;
    size_t offset;
    std::unordered_map<std::string_view, SourceLocation> tags;

    GrammarParser(ErrorReporter* er, std::string_view source);
    Grammar parse();

private:
    struct Directives {
        std::string_view start;
        std::string_view left_delim;
        std::string_view right_delim;
    };

    SourceLocation loc() const;

    int peek();
    int consume();
    bool eat(int c);
    void expect(int c);
    bool eat_delim();

    Directives directives();

    bool productions(std::vector<Production>& productions);
    void production(std::vector<Production>& productions);

    std::string_view word(); // [a-zA-Z_][a-zA-Z0-9]*
    std::string_view terminal(); // quoted word
    std::string_view tag(); // [word]
};

#endif
