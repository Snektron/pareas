#ifndef _PAREAS_LLPGEN_GRAMMAR_HPP
#define _PAREAS_LLPGEN_GRAMMAR_HPP

#include "pareas/llpgen/error_reporter.hpp"

#include <string>
#include <vector>
#include <iosfwd>
#include <stdexcept>
#include <string_view>
#include <cstddef>

struct InvalidGrammarError: std::runtime_error {
    InvalidGrammarError(const std::string& msg);
};

struct MultipleStartRulesError: InvalidGrammarError {
    MultipleStartRulesError();
};

struct InvalidStartRuleError: InvalidGrammarError {
    InvalidStartRuleError();
};

struct Terminal {
    std::string name;

    static Terminal null();
    bool is_null() const;
    bool operator==(const Terminal& other) const;
};

struct NonTerminal {
    std::string name;

    bool operator==(const NonTerminal& other) const;
};

struct Symbol {
    bool is_terminal;
    std::string name;

    Symbol(Terminal t);
    Symbol(NonTerminal nt);

    bool is_null() const;
    bool operator==(const Symbol& other) const;

    Terminal as_terminal() const;
    NonTerminal as_non_terminal() const;
};

struct Production {
    SourceLocation loc;
    std::string tag;
    NonTerminal lhs;
    std::vector<Symbol> rhs;
};

struct Grammar {
    Terminal left_delim;
    Terminal right_delim;

    const Production* start;
    std::vector<Production> productions;

    void dump(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const Terminal& t);
std::ostream& operator<<(std::ostream& os, const NonTerminal& nt);
std::ostream& operator<<(std::ostream& os, const Symbol& sym);
std::ostream& operator<<(std::ostream& os, const Production& prod);

template <>
struct std::hash<Terminal> {
    size_t operator()(const Terminal& t) const;
};

template <>
struct std::hash<NonTerminal> {
    size_t operator()(const NonTerminal& nt) const;
};

template <>
struct std::hash<Symbol> {
    size_t operator()(const Symbol& sym) const;
};

namespace literals {
    Terminal operator ""_t(const char* name, size_t len);
    NonTerminal operator ""_nt(const char* name, size_t len);
}

#endif
