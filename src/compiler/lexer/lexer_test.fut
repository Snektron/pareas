module lexer = import "lexer"
module test_lexer_grammar = import "test_lexer"

entry main [n] [m]
    (input: [n]u8)
    (initial_state: [256]u16)
    (merge_table: [m][m]u16)
    (final_state: [m]test_lexer_grammar.token.t)
    = lexer.lex input {
        initial_state = initial_state,
        merge_table = merge_table,
        final_state = final_state,
        identity_state = test_lexer_grammar.identity_state
    }
