#include "pareas/llpgen/llp/test_parser.hpp"

#include <fmt/ostream.h>

#include <stdexcept>

namespace llp {
    TestParser::TestParser(const ParsingTable* llp_table, std::span<const Terminal> input):
        llp_table(llp_table), input(input) {}

    bool TestParser::parse() {
        return this->compute_brackets() && this->verify_brackets();
    }

    void TestParser::dump(std::ostream& out) const {
        fmt::print(out, "Brackets:\n\t");
        for (const auto& bracket : brackets) {
            fmt::print(out, "{}{} ", bracket.side == BracketSide::LEFT ? "[" : "]", bracket.sym);
        }
        fmt::print(out, "\nDerivation:\n");
        for (const auto* prod : this->derivation) {
            fmt::print(out, "\t{}\n", *prod);
        }
    }

    bool TestParser::compute_brackets() {
        auto add_rbr = [&](const ParsingTable::Entry& entry) {
            auto syms = entry.initial_stack;
            for (auto it = syms.rbegin(); it != syms.rend(); ++it) {
                this->brackets.push_back({BracketSide::RIGHT, *it});
            }
        };

        auto add_lbr = [&](const ParsingTable::Entry& entry) {
            auto syms = entry.final_stack;
            for (auto it = syms.begin(); it != syms.end(); ++it) {
                this->brackets.push_back({BracketSide::LEFT, *it});
            }
        };

        auto add_derivation = [&](const ParsingTable::Entry& entry) {
            this->derivation.insert(this->derivation.end(), entry.productions.begin(), entry.productions.end());
        };

        for (size_t i = 1; i < this->input.size(); ++i) {
            auto ap = AdmissiblePair{
                this->input[i - 1],
                this->input[i],
            };

            auto it = this->llp_table->table.find(ap);
            if (it == this->llp_table->table.end())
                return false;

            add_rbr(it->second);
            add_lbr(it->second);
            add_derivation(it->second);
        }

        return true;
    }

    bool TestParser::verify_brackets() const {
        auto stack = std::vector<Symbol>();

        for (const auto& [side, sym] : this->brackets) {
            if (side == BracketSide::LEFT) {
                stack.push_back(sym);
            } else if (stack.empty() || stack.back() != sym) {
                return false;
            } else {
                stack.pop_back();
            }
        }

        return stack.empty();
    }
}
