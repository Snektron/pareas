import "lexer"
module text_lexer_grammar = import "test_lexer"
module test_lexer = lexer text_lexer_grammar

entry main [n] [m]
            (input: [n]u8)
            (initial_state: [256]u16)
            (merge_table: [m][m]u16)
            (final_state: [m]u8) =
    test_lexer.lex input initial_state merge_table final_state
