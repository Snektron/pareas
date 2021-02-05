#ifndef _PAREAS_LLPGEN_LR_PARSING_TABLE_HPP
#define _PAREAS_LLPGEN_LR_PARSING_TABLE_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/error_reporter.hpp"

#include <unordered_map>
#include <iosfwd>
#include <cstddef>

namespace lr {
    struct Action {
        enum class Type {
            SHIFT,
            REDUCE,
        };

        union Value {
            size_t state;
            const Production* reduction;
    };

        Type type;
        Value value;

        static Action shift(size_t state);
        static Action reduce(const Production* reduction);

        size_t as_shift() const;
        const Production* as_reduce() const;
    };

    std::ostream& operator<<(std::ostream& os, const Action& action);

    struct ActionKey {
        size_t state;
        Terminal lookahead;

        struct Hash {
            size_t operator()(const ActionKey& key) const;
        };
    };

    bool operator==(const ActionKey& lhs, const ActionKey& rhs);

    struct GotoKey {
        size_t state;
        NonTerminal nt;

        struct Hash {
            size_t operator()(const GotoKey& key) const;
        };
    };

    bool operator==(const GotoKey& lhs, const GotoKey& rhs);

    struct ConflictError: public InvalidGrammarError {
        ConflictError(): InvalidGrammarError("LR conflict: Grammar is not LR(1)") {}
    };

    struct ParsingTable {
        std::unordered_map<ActionKey, Action, ActionKey::Hash> action_table;
        std::unordered_map<GotoKey, size_t, GotoKey::Hash> goto_table;

        bool insert_action(const ActionKey& key, const Action& action);
        bool insert_goto(const GotoKey& key, size_t state);

        void dump_csv(std::ostream& os) const;
    };
}

#endif
