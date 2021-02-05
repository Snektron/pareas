#include "pareas/llpgen/ll/generator.hpp"

namespace ll {
    Generator::Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf):
        er(er), g(g), tsf(tsf) {}

    ParsingTable Generator::build_parsing_table() {
        auto ll = ParsingTable();
        bool error = false;

        auto insert = [&](const State& state, const Production* prod) {
            auto it = ll.table.find(state);
            if (it != ll.table.end()) {
                this->er->error_fmt(prod->loc, "LL parse conflict, grammar is not LL(1)");
                this->er->note(it->second->loc, "Conflicts with this production");

                error = true;
                return;
            }

            ll.table.insert(it, {state, prod});
        };

        for (const auto& prod : this->g->productions) {
            auto first = this->tsf->compute_first(prod.rhs);

            bool has_null = false;
            for (const auto& t : first) {
                if (t.is_null()) {
                    has_null = true;
                    continue;
                }

                insert({prod.lhs, t}, &prod);
            }

            if (has_null) {
                const auto& follow = this->tsf->follow(prod.lhs);
                for (const auto& t : follow) {
                    insert({prod.lhs, t}, &prod);
                }
            }
        }

        if (error)
            throw ConflictError();

        return ll;
    }
}
