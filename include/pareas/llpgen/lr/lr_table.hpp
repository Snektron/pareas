#ifndef _PAREAS_LLPGEN_LR_LR_TABLE_HPP
#define _PAREAS_LLPGEN_LR_LR_TABLE_HPP

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

    struct LRActionKey {
        size_t state;
        Terminal lookahead;

        struct Hash {
            size_t operator()(const LRActionKey& key) const;
        };
    };

    bool operator==(const LRActionKey& lhs, const LRActionKey& rhs);

    struct LRGotoKey {
        size_t state;
        NonTerminal nt;

        struct Hash {
            size_t operator()(const LRGotoKey& key) const;
        };
    };

    bool operator==(const LRGotoKey& lhs, const LRGotoKey& rhs);

    struct LRConflictError: public InvalidGrammarError {
        LRConflictError(): InvalidGrammarError("LR conflict: Grammar is not LR(1)") {}
    };

    struct LRTable {
        std::unordered_map<LRActionKey, Action, LRActionKey::Hash> action_table;
        std::unordered_map<LRGotoKey, size_t, LRGotoKey::Hash> goto_table;

        bool insert_action(const LRActionKey& key, const Action& action);
        bool insert_goto(const LRGotoKey& key, size_t state);

        void dump_csv(std::ostream& os) const;
    };
}

#endif
