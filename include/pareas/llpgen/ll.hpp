#ifndef _PAREAS_LLPGEN_LL_HPP
#define _PAREAS_LLPGEN_LL_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/error_reporter.hpp"

#include <unordered_map>
#include <vector>
#include <cstddef>

namespace ll {
    struct State {
        NonTerminal stack_top;
        Terminal lookahead;

        struct Hash {
            size_t operator()(const State& key) const;
        };
    };

    bool operator==(const State& lhs, const State& rhs);

    struct ConflictError: public InvalidGrammarError {
        ConflictError(): InvalidGrammarError("LL conflict: Grammar is not LL(1)") {}
    };

    struct ParsingTable {
        std::unordered_map<State, const Production*, State::Hash> table;

        std::vector<const Production*> partial_parse(const Terminal& y, std::vector<Symbol>& stack) const;
        void dump_csv(std::ostream& os) const;
    };

    class Generator {
        ErrorReporter* er;
        const Grammar* g;
        const TerminalSetFunctions* tsf;

    public:
        Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf);
        ParsingTable build_parsing_table();
    };
}

#endif
