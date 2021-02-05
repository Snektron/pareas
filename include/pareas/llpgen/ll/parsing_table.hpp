#ifndef _PAREAS_LLPGEN_LL_PARSING_TABLE_HPP
#define _PAREAS_LLPGEN_LL_PARSING_TABLE_HPP

#include "pareas/llpgen/grammar.hpp"

#include <vector>
#include <unordered_map>
#include <iosfwd>
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
}

#endif
