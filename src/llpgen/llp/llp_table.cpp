#include "pareas/llpgen/llp/llp_table.hpp"

#include <unordered_set>
#include <ostream>

namespace llp {
    void LLPTable::dump_csv(std::ostream& os) {
        // Print stacks in reverse to keep it the same as in the paper
        auto dump_syms_rev = [&](const auto& syms) {
            bool first = true;
            os << "{";
            for (auto it = syms.rbegin(); it != syms.rend(); ++it) {
                if (first)
                    first = false;
                else
                    os << " ";

                os << it->name;
            }
            os << "}";
        };

        auto dump_prods = [&](const auto& prods) {
            bool first = true;
            os << "{";
            for (const auto& prod : prods) {
                if (first)
                    first = false;
                else
                    os << ", ";

                os << *prod;
            }
            os << "}";
        };

        auto dump_entry = [&](const auto& entry) {
            // Custom print of symbols since we need to handle csv escapes
            os << "\"(";
            dump_syms_rev(entry.initial_stack);
            os << ", ";
            dump_syms_rev(entry.final_stack);
            os << ", ";
            dump_prods(entry.productions);
            os << ")\"";
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

                dump_entry(it->second);
            }
            os << std::endl;
        }
    }
}
