#ifndef _PAREAS_LPG_PARSER_GRAMMAR_HPP
#define _PAREAS_LPG_PARSER_GRAMMAR_HPP

#include "pareas/lpg/error_reporter.hpp"

#include <string>
#include <vector>
#include <iosfwd>
#include <stdexcept>
#include <string_view>
#include <cstddef>

namespace pareas::parser {
    struct InvalidGrammarError: std::runtime_error {
        InvalidGrammarError(const std::string& msg): std::runtime_error(msg) {}
    };

    struct Terminal {
        enum class Type {
            USER_DEFINED, // `name` contains a user defined string.
            EMPTY, // ε
            START_OF_INPUT, // ⊢
            END_OF_INPUT // ⊣
        };

        static const Terminal EMPTY;
        static const Terminal START_OF_INPUT;
        static const Terminal END_OF_INPUT;

        Type type;
        std::string name;

        bool is_empty() const;
        bool operator==(const Terminal& other) const;

        struct Hash {
            size_t operator()(const Terminal& t) const;
        };
    };

    struct NonTerminal {
        std::string name;

        bool operator==(const NonTerminal& other) const;

        struct Hash {
            size_t operator()(const NonTerminal& nt) const;
        };
    };

    struct Symbol {
        enum class Type {
            USER_DEFINED_TERMINAL,
            EMPTY_TERMINAL,
            START_OF_INPUT_TERMINAL,
            END_OF_INPUT_TERMINAL,
            NON_TERMINAL,
        };

        Type type;
        std::string name;

        Symbol(Terminal t);
        Symbol(NonTerminal nt);

        bool is_empty_terminal() const;
        bool is_terminal() const;
        bool operator==(const Symbol& other) const;

        Terminal as_terminal() const;
        NonTerminal as_non_terminal() const;

        struct Hash {
            size_t operator()(const Symbol& sym) const;
        };
    };

    struct Production {
        SourceLocation loc;
        std::string tag;
        NonTerminal lhs;
        std::vector<Symbol> rhs;

        size_t arity() const;
    };

    struct Grammar {
        constexpr const static size_t START_INDEX = 0;

        std::vector<Production> productions;

        void dump(std::ostream& os) const;
        void validate(ErrorReporter& er) const;
        const Production* start() const;

    private:
        bool check_production_definitions(ErrorReporter& er) const;
        bool check_start_rule(ErrorReporter& er) const;
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

#endif
