module lexer = import "lexer"
module test_lexer_grammar = import "test_lexer"

entry main [n] [m]
    (input: [n]u8)
    (initial_state: [256]lexer.state)
    (merge_table: [m][m]lexer.state)
    (final_state: [m]test_lexer_grammar.token.t) =
    let lt =
        lexer.mk_lex_table
            initial_state
            merge_table
            final_state
            test_lexer_grammar.identity_state
    in lexer.lex input lt
