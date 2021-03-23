# Control keywords
fn = /fn/
if = /if/
else = /else/
while = /while/
return = /return/

# Type keywords
float = /float/
int = /int/
void = /void/

# Operators
eq = /=/
plus = /\+/
minus = /-/
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
exclam = /!/
tilde = /~/
semi = /;/

# Parenthesis
rparen = /\)/
lparen = /\(/
rbracket = /\]/
lbracket = /\[/
rbrace = /}/
lbrace = /{/

# Semantic: These tokens carry information that is required for the rest of the compilation.
id = /[a-zA-Z_][a-zA-Z0-9_]*/
float_literal = /[0-9]+.[0-9]+/
int_literal = /[0-9]+/

# Ignored: These should be filtered out before parsing.
whitespace = /[ \t\r\n]+/
comment = /\/\/[^\n]*\n/

# Hacks
binary_minus = /-/ [rparen, id, float_literal, int_literal]
