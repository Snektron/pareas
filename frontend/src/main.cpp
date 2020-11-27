#include <iostream>
#include <ostream>

#include "grammar.hpp"
#include "trie.hpp"
#include "opg.hpp"
#include "opp.hpp"

enum class NonTerminal {
    E, T, F
};

enum class Terminal {
    LPAREN, RPAREN, A, STAR, PLUS, DELIM,
};

std::ostream& operator<<(std::ostream& os, NonTerminal nt) {
    switch (nt) {
        case NonTerminal::E:
            os << 'E';
            break;
        case NonTerminal::T:
            os << 'T';
            break;
        case NonTerminal::F:
            os << 'F';
            break;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, Terminal t) {
    switch (t) {
        case Terminal::LPAREN:
            os << '(';
            break;
        case Terminal::RPAREN:
            os << ')';
            break;
        case Terminal::A:
            os << 'a';
            break;
        case Terminal::STAR:
            os << '*';
            break;
        case Terminal::PLUS:
            os << '+';
            break;
        case Terminal::DELIM:
            os << '$';
    }

    return os;
}

auto main() -> int {
    using G = Grammar<NonTerminal, Terminal>;

    // example of 'The Theory of Parsing, Translation and Compiling' (p 439)
    auto test_grammar = G{
        .start = NonTerminal::E,
        .delim = Terminal::DELIM,
        .productions = {
            {NonTerminal::E, {NonTerminal::E, Terminal::PLUS, NonTerminal::T}},
            {NonTerminal::E, {NonTerminal::T}},
            {NonTerminal::T, {NonTerminal::T, Terminal::STAR, NonTerminal::F}},
            {NonTerminal::T, {NonTerminal::F}},
            {NonTerminal::F, {Terminal::LPAREN, NonTerminal::E, Terminal::RPAREN}},
            {NonTerminal::F, {Terminal::A}},
        }
    };

    auto opg = OperatorPrecedenceGrammar(&test_grammar);
    auto pm = opg.build_precedence_matrix();

    // Just to print it similarly to the example
    auto rorder = {Terminal::LPAREN, Terminal::A, Terminal::STAR, Terminal::PLUS, Terminal::RPAREN, Terminal::DELIM};
    auto lorder = {Terminal::RPAREN, Terminal::A, Terminal::STAR, Terminal::PLUS, Terminal::LPAREN, Terminal::DELIM};

    std::cout << "Precedence Matrix:\n ";
    for (auto r : rorder) {
        std::cout << " " << r;
    }
    std::cout << std::endl;

    for (auto l : lorder) {
        std::cout << l;
        for (auto r : rorder) {
            auto it = pm.find({l, r});
            if (it == pm.end()) {
                std::cout << "  ";
                continue;
            }

            switch (it->second) {
                case PrecedenceOrder::LESS:
                    std::cout << " <";
                    break;
                case PrecedenceOrder::EQUAL:
                    std::cout << " =";
                    break;
                case PrecedenceOrder::GREATER:
                    std::cout << " >";
                    break;
            }
        }

        std::cout << std::endl;
    }

    auto [f, g] = opg.build_precedence_functions(pm);

    std::cout << "\nPrecedence functions:\n";

    for (auto t : lorder) {
        std::cout << t << " " << f[t] << " " << g[t] << std::endl;
    }

    std::cout << "\nPrecedence matrix according to precedence functions:\n ";
    for (auto r : rorder) {
        std::cout << " " << r;
    }
    std::cout << std::endl;

    for (auto l : lorder) {
        std::cout << l;
        for (auto r : rorder) {
            if (f[l] < g[r]) {
                std::cout << " <";
            } else if (f[l] == g[r]) {
                std::cout << " =";
            } else {
                std::cout << " >";
            }
        }

        std::cout << std::endl;
    }

    auto opp = OperatorPrecedenceParser<G>{.grammar = &test_grammar, .f = &f, .g = &g};

    auto test_input = {
        Terminal::LPAREN,
        Terminal::A,
        Terminal::PLUS,
        Terminal::A,
        Terminal::RPAREN,
    };

    auto result = opp.parse(std::size(test_input), std::data(test_input));
    std::cout << (result ? "accept" : "reject") << std::endl;

    return 0;
}