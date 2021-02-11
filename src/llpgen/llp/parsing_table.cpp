#include "pareas/llpgen/llp/parsing_table.hpp"

#include <fmt/ostream.h>

#include <unordered_set>
#include <ostream>

namespace llp {
    void ParsingTable::dump_csv(std::ostream& os) {
        // Print stacks in reverse to keep it the same as in the paper
        auto dump_syms_rev = [&](const auto& syms) {
            bool first = true;
            fmt::print(os, "{{");
            for (auto it = syms.rbegin(); it != syms.rend(); ++it) {
                if (first)
                    first = false;
                else
                    fmt::print(os, " ");
                fmt::print(os, "{}", it->name);
            }
            fmt::print(os, "}}");
        };

        auto dump_prods = [&](const auto& prods) {
            bool first = true;
            fmt::print(os, "{{");
            for (const auto& prod : prods) {
                if (first)
                    first = false;
                else
                    fmt::print(os, ", ");
                fmt::print(os, "{}", *prod);
            }
            fmt::print(os, "}}");
        };

        auto dump_entry = [&](const auto& entry) {
            // Custom print of symbols since we need to handle csv escapes
            fmt::print(os, "\"(");
            dump_syms_rev(entry.initial_stack);
            fmt::print(os, ", ");
            dump_syms_rev(entry.final_stack);
            fmt::print(os, ", ");
            dump_prods(entry.productions);
            fmt::print(os, ")\"");
        };

        auto ys = std::unordered_set<Terminal>();
        auto xs = std::unordered_set<Terminal>();

        for (const auto& [ap, gamma] : this->table) {
            const auto& [x, y] = ap;
            xs.insert(x);
            ys.insert(y);
        }

        for (const auto& y : ys) {
            fmt::print(os, ",{}", y);
        }
        fmt::print(os, "\n");

        for (const auto& x : xs) {
            fmt::print(os, "{}", x);
            // Hope that this iterates in the same order
            for (const auto& y : ys) {
                fmt::print(os, ",", y);
                auto it = this->table.find({x, y});
                if (it == this->table.end())
                    continue;

                dump_entry(it->second);
            }
            fmt::print(os, "\n");
        }
    }
}
