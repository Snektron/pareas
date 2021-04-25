## Control keywords
fn = /fn/
if = /if/
else = /else/
elif = /elif/
while = /while/
return = /return/
var = /var/

## Type keywords
float = /float/
int = /int/
void = /void/

## Operators
eq = /=/
plus = /\+/
unary_minus = /-/
star = /\*/
slash = /\//
percent = /%/
and = /&/
pipe = /\|/
hat = /^/
lt_lt = /<</
gt_gt = />>/
gt_gt_gt = />>>/
and_and = /&&/
pipe_pipe = /\|\|/
eq_eq = /==/
neq = /!=/
gt = />/
gte = />=/
lt = /</
lte = /<=/
exclaim = /!/ #
tilde = /~/ #
semi = /;/
comma = /,/
colon = /:/

# The parser cannot differ between unary and binary minus, as it accepts a subset of LL.
# For this case, we differentiate binary minus from unary minus based on the tokens
# that precede it.
# Also define a special binary whitespace token so that we can lex `a - b`.
binary_minus_whitespace = /[ \t\r\n]+/ [rparen, name, float_literal, int_literal]
binary_minus = /-/ [rparen, name, float_literal, int_literal, binary_minus_whitespace]

## Parenthesis
rparen = /\)/
lparen = /\(/
rbracket = /\]/
lbracket = /\[/
rbrace = /}/
lbrace = /{/

## Semantic: These tokens carry information that is required for the rest of the compilation.
name = /[a-zA-Z_][a-zA-Z0-9_]*/
float_literal = /[0-9]+.[0-9]+/
int_literal = /[0-9]+/

## Ignored: These should be filtered out before parsing.
whitespace = /[ \t\r\n]+/
comment = /\/\/[^\n]*\n/
