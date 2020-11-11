#ifndef _PAREAS_OPP_HPP
#define _PAREAS_OPP_HPP

#include "opg.hpp"
#include <vector>
#include <cstdint>
#include <iostream>
#include <optional>

template <typename Grammar>
struct OperatorPrecedenceParser {
    using Terminal = typename Grammar::Terminal;
    using NonTerminal = typename Grammar::NonTerminal;
    using Symbol = typename Grammar::Symbol;
    using PrecedenceFunction = typename OperatorPrecedenceGrammar<Grammar>::PrecedenceFunction;

    const Grammar* grammar;
    PrecedenceFunction* f;
    PrecedenceFunction* g;

    template <typename Iter>
    auto find_reduction(Iter begin, Iter end) -> std::optional<NonTerminal>;

    auto parse(size_t n, const Terminal* input) -> bool;
};

template <typename Grammar>
template <typename Iter>
auto OperatorPrecedenceParser<Grammar>::find_reduction(Iter begin, Iter end) -> std::optional<NonTerminal> {
    for (auto& [nt, prod] : this->grammar->productions) {
        auto a = begin;
        auto b = end;

        if (std::equal(a, b, prod.begin(), prod.end())) {
            return nt;
        }
    }

    return std::nullopt;
}

template <typename Grammar>
auto OperatorPrecedenceParser<Grammar>::parse(size_t n, const Terminal* input) -> bool {
    auto stack = std::vector<Symbol>();
    stack.push_back(this->grammar->delim);

    auto dump_prod = [](auto& prod) {
        for (auto& sym : prod) {
            if (auto* t = std::get_if<Terminal>(&sym)) {
                std::cout << " " << *t;
            } else {
                std::cout << " " << std::get<NonTerminal>(sym);
            }
        }
    };

    size_t i = 0;
    while (true) {
    start:
        if (i == n && !stack.empty() && stack.back() == Symbol(this->grammar->delim)) {
            return true;
        }

        auto it = std::find_if(stack.rbegin(), stack.rend(), [](auto& sym){ return std::holds_alternative<Terminal>(sym); });
        auto a = std::get<Terminal>(*it);
        auto b = i == n ? this->grammar->delim : input[i];

        if ((*this->f)[a] <= (*this->g)[b]) {
            stack.push_back(b);
            std::cout << "shift " << b << std::endl;
            ++i;
        } else {
            auto prev = a;
            auto prod = std::vector<Symbol>();

            while (--stack.rbegin() != it) {
                prod.push_back(stack.back());
                stack.pop_back();
            }

            while (!stack.empty()) {
                auto top = stack.back();

                if (auto* t = std::get_if<Terminal>(&top)) {
                    if ((*this->f)[*t] < (*this->g)[prev]) {
                        std::reverse(prod.begin(), prod.end());
                        std::cout << "reduce: ";
                        dump_prod(prod);
                        auto nt = this->find_reduction(prod.begin(), prod.end());

                        if (!nt.has_value()) {
                            std::cout << " -> {invalid}" << std::endl;
                            return false;
                        } else {
                            std::cout << " -> " << nt.value() << std::endl;
                        }

                        stack.push_back(Symbol(nt.value()));

                        goto start;
                    }

                    prev = *t;
                }

                prod.push_back(top);
                stack.pop_back();
            }

            return false;
        }
    }
}

#endif
