#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/grammar_parser.hpp"
#include "pareas/llpgen/generator.hpp"

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
    auto parser = GrammarParser(test_grammar);

    try {
       auto g = parser.parse();
       auto gen = LLPGenerator(&g);
       auto psls = gen.build_psls_table();
       auto ll = gen.build_ll_table();
       auto llp = gen.build_llp_table(ll, psls);
       llp.dump_csv(std::cout);
    } catch (const ParseError& e) {
        std::cerr << "Parse error at " << (parser.line + 1) << ":"
            << (parser.column + 1) << ": " << e.what() << std::endl;
        std::cerr << parser.current_line() << std::endl;

        for (size_t i = 0; i < parser.column; ++i) {
            std::cerr << ' ';
        }
        std::cerr << '^' << std::endl;

        return EXIT_FAILURE;
    } catch (const InvalidGrammarError& e) {
        std::cerr << "Grammar error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
