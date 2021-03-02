#include <fmt/format.h>
#include <fmt/ostream.h>

#include "pareas/common/error_reporter.hpp"
#include "pareas/common/parser.hpp"
#include "pareas/lexgen/lexer_parser.hpp"
#include "pareas/lexgen/fsa.hpp"
#include "pareas/lexgen/parallel_lexer.hpp"
#include "pareas/lexgen/render.hpp"

#include <iostream>
#include <cstdlib>

auto test_input = R"HERE(
float = /float/
int = /int/
struct = /struct/
unsigned = /unsigned/
else = /else/
for = /for/
void = /void/
if = /if/
return = /return/
static = /static/
while = /while/
rparen = /\(/
lparen = /\)/
rbracket = /\[/
lbracket = /\]/
rbrace = /{/
lbrace = /}/
plus = /\+/
minus = /-/
slash = /\//
star = /\*/
equals = /=/
lt = /</
semi = /;/
id = /[a-zA-Z_][a-zA-Z0-9_]*/
number = /[0-9]+/
whitespace = /[ \r\n][ \t\r\n]*/
comment = /\/\/[^\n]*\n/
)HERE";

int main() {
    auto er = pareas::ErrorReporter(test_input, std::clog);
    auto parser = pareas::Parser(&er, test_input);
    auto lexer_parser = pareas::LexerParser(&parser);
    auto tokens = lexer_parser.parse();
    auto parallel_lexer = pareas::ParallelLexer(tokens);
    auto renderer = pareas::LexerRenderer(tokens, &parallel_lexer);
    // renderer.render_token_definitions(std::cout);
    // renderer.render_initial_state_dataset(std::cout);
    // renderer.render_merge_table_dataset(std::cout);
    renderer.render_final_state_dataset(std::cout);

    return EXIT_SUCCESS;
}
