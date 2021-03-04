#ifndef _PAREAS_LLPGEN_GRAMMAR_HPP
#define _PAREAS_LLPGEN_GRAMMAR_HPP

#include "pareas/common/error_reporter.hpp"

#include <string>
#include <vector>
#include <iosfwd>
#include <stdexcept>
#include <string_view>
#include <cstddef>

namespace pareas {
    struct InvalidGrammarError: std::runtime_error {
        InvalidGrammarError(const std::string& msg): std::runtime_error(msg) {}
    };

    struct MultipleStartRulesError: InvalidGrammarError {
        MultipleStartRulesError(): InvalidGrammarError("Start rule appears in multiple productions") {}
    };

    struct InvalidStartRuleError: InvalidGrammarError {
        InvalidStartRuleError(): InvalidGrammarError("Start rule is not in right form") {}
    };

    struct MissingRuleDefinitionError: InvalidGrammarError {
        MissingRuleDefinitionError(): InvalidGrammarError("Missing definition for rule") {}
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
        constexpr const static size_t START_INDEX = 0;

        Terminal left_delim;
        Terminal right_delim;

        std::vector<Production> productions;

        void dump(std::ostream& os) const;
        void validate(ErrorReporter& er) const;
        const Production* start() const;
    };

    std::ostream& operator<<(std::ostream& os, const Terminal& t);
    std::ostream& operator<<(std::ostream& os, const NonTerminal& nt);
    std::ostream& operator<<(std::ostream& os, const Symbol& sym);
    std::ostream& operator<<(std::ostream& os, const Production& prod);

    namespace literals {
        Terminal operator ""_t(const char* name, size_t len);
        NonTerminal operator ""_nt(const char* name, size_t len);
    }
}

template <>
struct std::hash<pareas::Terminal> {
    size_t operator()(const pareas::Terminal& t) const;
};

template <>
struct std::hash<pareas::NonTerminal> {
    size_t operator()(const pareas::NonTerminal& nt) const;
};

template <>
struct std::hash<pareas::Symbol> {
    size_t operator()(const pareas::Symbol& sym) const;
};

#endif
