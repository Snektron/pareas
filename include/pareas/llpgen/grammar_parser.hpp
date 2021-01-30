#ifndef _PAREAS_LLPGEN_GRAMMAR_PARSER_HPP
#define _PAREAS_LLPGEN_GRAMMAR_PARSER_HPP

#include "pareas/llpgen/grammar.hpp"

#include <string_view>
#include <variant>
#include <unordered_set>
#include <cstddef>

struct ParseError: InvalidGrammarError {
    ParseError(const std::string& message);
};

struct GrammarParser {
    std::string_view source;
    size_t offset;
    size_t line;
    size_t column;
    std::unordered_set<std::string_view> tags;

    GrammarParser(std::string_view source);
    Grammar parse();
    std::string_view current_line() const;

private:
    struct Directives {
        std::string_view start;
        std::string_view left_delim;
        std::string_view right_delim;
    };

    int peek();
    int consume();
    bool eat(int c);
    void expect(int c);
    bool eat_delim();

    Directives directives();

    void productions(Grammar& g);
    void production(Grammar& g);

    std::string_view word(); // [a-zA-Z_][a-zA-Z0-9]*
    std::string_view terminal(); // quoted word
    std::string_view tag(); // [word]
};

#endif
