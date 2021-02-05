#include "pareas/llpgen/error_reporter.hpp"
#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/grammar_parser.hpp"
#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/llp/llp_generator.hpp"
#include "pareas/llpgen/llp/test_parser.hpp"
#include "pareas/llpgen/ll.hpp"
#include "pareas/llpgen/lr/lr_generator.hpp"

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

auto lr_test_grammar = R"(
%start = start;
%left_delim = 'soi';
%right_delim = 'eoi';

start -> 'soi' expr 'eoi';

expr [expr_sum] -> expr 'plus' atom;
expr [expr_atom] -> atom;

atom [literal] -> 'a';
atom [brackets] -> 'lbracket' expr 'rbracket';
)";

int main() {
    auto er = ErrorReporter(lr_test_grammar, std::clog);
    auto parser = GrammarParser(&er, lr_test_grammar);

    try {
       auto g = parser.parse();
       auto tsf = TerminalSetFunctions(g);
       // tsf.dump(std::cout);

       auto gen = lr::LRGenerator(&er, &g, &tsf);
       auto lr_table = gen.build_lr_table();
       // gen.dump(std::cout);

       lr_table.dump_csv(std::cout);

       // auto gen = llp::LLPGenerator(&er, &g, &tsf);
       // auto psls_table = gen.build_psls_table();
       // psls_table.dump_csv(std::cout);

       // auto ll_table = ll::Generator(&er, &g, &tsf).build_parsing_table();
       // // ll_table.dump_csv(std::cout);

       // auto llp_table = gen.build_llp_table(ll_table, psls_table);
       // // llp_table.dump_csv(std::cout);

       // auto input = {"soi"_t, "a"_t, "plus"_t, "lbracket"_t, "a"_t, "plus"_t, "a"_t, "rbracket"_t, "eoi"_t};
       // auto test_parser = llp::TestParser(&llp_table, input);
       // auto success = test_parser.parse();
       // std::cout << "Parsing " << (success ? "success" : "failed") << std::endl;

       // test_parser.dump(std::cout);
    } catch (const InvalidGrammarError& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
