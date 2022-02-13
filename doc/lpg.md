# Lexer- and Parser Generator

`pareas-lpg` implements the lexical analyzer and parser generator used by this project. It is used in specific to generate the lexical analyzers and parsers for both the compiler and the json parser, but are designed as general tools. This tool mostly generates data tables that are to be used by the lexer and parser runtime, found in`src/compiler/lexer/lexer.fut` and `src/compiler/parser/` for the lexer and parser respectively.

## Lexical Analyzer

Lexical analyzers are described by a lexical grammar definition, a single file mapping lexeme identifiers to regular expressions. These are compiled into a single automaton, which when matched produces a lexeme stream as if repeatedly simulating all regular expressions on the start of the input. The resulting lexeme is produced from the regular expression which matched the most characters. If two match exactly the same amount of characters, the lexeme that is defined earlier in the file is produced.

The structure of such a lexical grammar definition file consists of a number of lexeme-and-regular-expression pairs, written using the syntax `<lexeme name> = /<regex>/`. `<lexeme name>` is an identifying name for the lexeme, which may consist of alphanumeric characters and `_`. This identifier is required to be unique in the lexical grammar definition. `<regex>` is a regular expression that describes the character sequence which should be associated with that lexeme. Comments are written using `#`, anything on the same line after `#` is ignored. Comments cannot be started while inside a regular expression, in that case `#` is literal.

### Regular expressions

`pareas-lpg` supports the following regex subset:

| Syntax   | Description                                                                       |
| ------   | --------------------------------------------------------------------------------- |
| `a|b`    | Alternation: Match either sub-regex `a` or `b`.                                   |
| `ab`     | Concatenation: First match sub-regex `a`, then `b`.                               |
| `a*`     | Zero-or-more: Match sub-regex `a` any number of times.                            |
| `a+`     | One-or-more: Match sub-regex `a` any number of times, but at least once.          |
| `a?`     | Zero-or-one: Match sub-regex `a` once or not at all.                              |
| `(a)`    | Group: Group sub-regex `a` together. Useful in combination with `*`, `+` and `?`. |
| `.`      | Match any character.                                                              |
| `[a-bc]` | Match character set.                                                              |
| `\x`     | Escape: Insert a special character.                                               |
| other    | Literally match character.                                                        |

The sequence between the brackets of a character set defines the set of characters that may be matched: Either of the characters are accepted. This construct accepts the special syntax `a-b`, which matches any character `a` through `b`, in ascii-order. Escaped characters are also allowed.

### Escape sequences

The following escape sequences are allowed:

| Syntax | Description                        |
| ------ | ---------------------------------- |
| `\n`   | Newline                            |
| `\r`   | Carriage return                    |
| `\t`   | Tabulation                         |
| `\\`   | Backslash                          |
| `\'`   | Single quote                       |
| `\"`   | Double quote                       |
| `\-`   | Minus (useful in character sets )  |
| `\^`   | Hat                                |
| `\xAA` | Hexadecimal interpretation of `AA` |

### Precede sets

The lexical analyzer supports a rudimentary method to limit the context in which a particular lexeme may be matched. The syntax `<lexeme name> = /<regex>/ [<precede list...>]`, where `<precede list...>` is a comma-separated list of unique lexeme names, yields a lexical analyzer where the lexeme `<lexeme name>` may only be matched after any of the lexemes in `<precede list...>`. Note that this lexeme is given priority over normal tokens. This allows the lexer to identify some tokens in a way that reduces the complexity of the parsing grammar, for example in the following construct:
```
int_literal = /[0-9]+/
plus = /+/
unary_minus = /-/
# If the parser cannot decide whether a minus is binary or unary, the lexical analyzer can
# be used to tell the difference: any minus which follows an int literal is binary, and any
# other situation is unary. Note that after `int_literal` has been matched, `binary_minus`
# is given priority over `unary_minus`.
binary_minus = /-/ [int_literal]
```

## Parser Generator

Similar to lexical analyzers, parsers are defined by a grammar definition. In this case, productions are mapped to a sequence of symbols, which may include either lexemes or other productions. Matching starts at the first defined production, and recursively expands until it matches the entire input. The particular algorithm for the parser is described by Vagner & Melichar in "Parallel LL Parsing". `pareas-lpg` implements an `LLP(1, 1)` parser generator.

Productions are written using the syntax `<name> [<tag>] -> <symbols...>;`. `<name>` is an identifying name for the production, by which it may later be referenced. Several productions with the same `<name>` may be defined; in this case, the parser may match either of the productions when the corresponding `<name>` is expanded. `<tag>` is an identifying tag for the production, which must be unique in the grammar definition. This tag can later be used to tell which production was actually matched. Both `<name>` and `<tag>` take the form of a character sequence consisting of alphanumeric characters and `_`. `<symbols...>` is a sequence of zero-or-more lexemes and productions, which are matched from right to left. To match a lexeme, one may use the syntax `'<lexeme>'`. To match another production, one may use the syntax `<production name>` instead. The shorthand `<name> -> <symbols...>;` may be used to define a production where the `<name>` is equal to the `<tag>`. Comments are written using `#`, in the same way as is done in the lexical analyzer.
```
expr -> atom sum;
sum [sum_add] -> 'plus' atom sum;
sum [sum_sub] -> 'minus' atom sum;
sum [sum_end] -> ;
atom [atom_name] -> 'name';
atom [atom_number] -> 'number';
atom [atom_paren] -> 'lparen' expr 'rparen';
```
