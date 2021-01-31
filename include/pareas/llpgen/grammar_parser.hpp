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

class GrammarParser {
    struct Directive {
        std::string_view value;
        SourceLocation loc;
    };

    ErrorReporter* er;
    std::string_view source;
    size_t offset;
    std::vector<Production> productions;
    std::unordered_map<std::string_view, SourceLocation> tags;
    Directive start, left_delim, right_delim;

public:
    GrammarParser(ErrorReporter* er, std::string_view source);
    Grammar parse();

private:
    SourceLocation loc() const;

    int peek();
    int consume();
    bool eat(int c);
    bool expect(int c);
    bool eat_delim();
    void skip_statement();

    bool directive();
    bool production();

    std::string_view word(); // [a-zA-Z_][a-zA-Z0-9]*
    std::string_view terminal(); // quoted word
    std::string_view tag(); // [word]
};

#endif
