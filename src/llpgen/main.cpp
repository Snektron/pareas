#include "pareas/llpgen/error_reporter.hpp"
#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/grammar_parser.hpp"
#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/ll/generator.hpp"
#include "pareas/llpgen/llp/generator.hpp"
#include "pareas/llpgen/llp/render.hpp"
#include "pareas/llpgen/llp/test_parser.hpp"

#include <fmt/ostream.h>

#include <iostream>

using literals::operator ""_t;
using literals::operator ""_nt;

auto test_grammar = R"(
%start = start;
%left_delim = 'soi';
%right_delim = 'eoi';

start -> 'soi' expr 'eoi';

expr -> atom sum;

sum [sum] -> 'plus' atom sum;
sum [sum_end] -> ;

atom [literal] -> 'a';
atom [brackets] -> 'lbracket' expr 'rbracket';
)";

int main() {
    auto er = ErrorReporter(test_grammar, std::clog);
    auto parser = GrammarParser(&er, test_grammar);

    try {
        auto g = parser.parse();
        auto tsf = TerminalSetFunctions(g);
        // tsf.dump(std::cout);

        auto gen = llp::Generator(&er, &g, &tsf);
        auto psls_table = gen.build_psls_table();
        // psls_table.dump_csv(std::cout);

        auto ll_table = ll::Generator(&er, &g, &tsf).build_parsing_table();
        // ll_table.dump_csv(std::cout);

        auto parsing_table = gen.build_parsing_table(ll_table, psls_table);
        // llp_table.dump_csv(std::cout);

        llp::render_parser(std::cout, g, parsing_table);

        // auto input = {"soi"_t, "a"_t, "plus"_t, "lbracket"_t, "a"_t, "plus"_t, "a"_t, "rbracket"_t, "eoi"_t};
        // auto test_parser = llp::TestParser(&parsing_table, input);
        // auto success = test_parser.parse();
        // std::cout << "Parsing " << (success ? "success" : "failed") << std::endl;

        // test_parser.dump(std::cout);
    } catch (const InvalidGrammarError& e) {
        fmt::print(std::cerr, "Failed: {}\n", e.what());
        return EXIT_FAILURE;
    }
}
