import "parser"
module lexer = import "../lexer/lexer"

module grammar = import "better_grammar"
module better_parser = parser grammar

let main [n] [m]
    (input: [n]u8)
    (initial_state: [256]lexer.state)
    (merge_table: [m][m]lexer.state)
    (final_state: [m]grammar.token.t) =
    let lt =
        lexer.mk_lex_table
            initial_state
            merge_table
            final_state
            grammar.identity_state
    let tokens =
        lexer.lex input lt -- TODO: Check for errors
        |> filter (!= grammar.token_whitespace)
    in if !(better_parser.check tokens) then -1 else
    let parse = better_parser.parse tokens
    let parents = better_parser.build_parent_vector parse
    in last parents
