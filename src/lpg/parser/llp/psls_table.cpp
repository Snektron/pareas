#include "pareas/lpg/parser/llp/psls_table.hpp"

#include <fmt/ostream.h>

#include <unordered_set>

namespace pareas::llp {
    void PSLSTable::dump_csv(std::ostream& os) const {
        auto dump_syms = [&](const auto& syms) {
            // Custom print of symbols since we need to handle csv escapes
            fmt::print(os, "\"");
            bool first = true;
            for (const auto& sym : syms) {
                if (first)
                    first = false;
                else
                    fmt::print(os, " ");
                fmt::print(os, "{}", sym.name);
            }

            fmt::print(os, "\"");
        };

        auto ys = std::unordered_set<Terminal>();
        auto xs = std::unordered_set<Terminal>();

        for (const auto& [ap, gamma] : this->table) {
            const auto& [x, y] = ap;
            xs.insert(x);
            ys.insert(y);
        }

        for (const auto& y : ys) {
            fmt::print(",{}", y);
        }
        fmt::print("\n");

        for (const auto& x : xs) {
            fmt::print(os, "{}", x);
            // Hope that this iterates in the same order
            for (const auto& y : ys) {
                fmt::print(os, ",");
                auto it = this->table.find({x, y});
                if (it == this->table.end())
                    continue;

                dump_syms(it->second.gamma);
            }
            fmt::print(os, "\n");
        }
    }
}
