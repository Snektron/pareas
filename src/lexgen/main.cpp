#include <fmt/format.h>
#include <fmt/ostream.h>

#include "pareas/common/error_reporter.hpp"
#include "pareas/common/parser.hpp"
#include "pareas/lexgen/lexer_parser.hpp"
#include "pareas/lexgen/fsa.hpp"
#include "pareas/lexgen/parallel_lexer.hpp"

#include <iostream>
#include <cstdlib>

auto test_input = R"HERE(
const = /const/
double = /double/
float = /float/
int = /int/
short = /short/
struct = /struct/
unsigned = /unsigned/
break = /break/
continue = /continue/
else = /else/
for = /for/
long = /long/
signed = /signed/
switch = /switch/
void = /void/
case = /case/
default = /default/
enum = /enum/
goto = /goto/
register = /register/
sizeof = /sizeof/
typedef = /typedef/
volatile = /volatile/
char = /char/
do = /do/
extern = /extern/
if = /if/
return = /return/
static = /static/
union = /union/
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

    return EXIT_SUCCESS;
}
