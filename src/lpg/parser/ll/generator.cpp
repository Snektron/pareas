#include "pareas/lpg/parser/ll/generator.hpp"

namespace pareas::parser::ll {
    Generator::Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf):
        er(er), g(g), tsf(tsf) {}

    ParsingTable Generator::build_parsing_table() {
        auto ll = ParsingTable();
        bool error = false;

        auto insert = [&](const State& state, const Production* prod) {
            auto it = ll.table.find(state);
            if (it != ll.table.end()) {
                this->er->error(prod->loc, "LL parse conflict, grammar is not LL(1)");
                this->er->note(it->second->loc, "Conflicts with this production");

                error = true;
                return;
            }

            ll.table.insert(it, {state, prod});
        };

        for (const auto& prod : this->g->productions) {
            auto first = this->tsf->compute_first(prod.rhs);

            bool has_empty = false;
            for (const auto& t : first) {
                if (t.is_empty()) {
                    has_empty = true;
                    continue;
                }

                insert({prod.lhs, t}, &prod);
            }

            if (has_empty) {
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
