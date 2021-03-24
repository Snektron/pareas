import "parser/parser"
module lexer = import "lexer/lexer"
module grammar = import "../../gen/pareas_grammar"
module pareas_parser = parser grammar

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
    let (tokens, _, _) =
        lexer.lex input lt
        --  |> filter (\(t, _, _) -> t != grammar.token_whitespace && t != grammar.token_comment)
        |> unzip3
    in tokens
    -- As the lexer returns an `invalid` token when the input cannot be lexed, which is accepted
    -- by the parser also, pareas_parser.check will fail whenever there is a lexing error.
    --  in if !(pareas_parser.check tokens) then -1 else
    --  let parse = pareas_parser.parse tokens
    --  let parents = pareas_parser.build_parent_vector parse
    --  in last parents