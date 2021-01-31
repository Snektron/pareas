#include "pareas/llpgen/llp/psls_table.hpp"

#include <unordered_set>
#include <ostream>

namespace llp {
    void PSLSTable::dump_csv(std::ostream& os) const {
        auto dump_syms = [&](const auto& syms) {
            // Custom print of symbols since we need to handle csv escapes
            os << '"';
            bool first = true;
            for (const auto& sym : syms) {
                if (first)
                    first = false;
                else
                    os << " ";

                os << sym.name;
            }

            os << '"';
        };

        auto ys = std::unordered_set<Terminal>();
        auto xs = std::unordered_set<Terminal>();

        for (const auto& [ap, gamma] : this->table) {
            const auto& [x, y] = ap;
            xs.insert(x);
            ys.insert(y);
        }

        for (const auto& y : ys) {
            os << "," << y;
        }
        os << std::endl;

        for (const auto& x : xs) {
            os << x;
            // Hope that this iterates in the same order
            for (const auto& y : ys) {
                os << ",";
                auto it = this->table.find({x, y});
                if (it == this->table.end())
                    continue;

                dump_syms(it->second.gamma);
            }
            os << std::endl;
        }
    }
}
