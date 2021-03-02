module type lexer_grammar = {
    module token: integral
    val num_tokens: i64
    val identity_state: u16
}

module lexer (g: lexer_grammar) = {
    type state = u16
    let produces_token_mask: state = 0x8000

    let reject_state: state = 0
    let start_state: state = 1

    let lex [n] [m] (input: [n]u8) (initial_state: [256]u16) (merge_table: [m][m]u16)
                (final_state: [m]u8) =
        let merge (a: state) (b: state) =
            let a = a & !produces_token_mask
            let b = b & !produces_token_mask
            in merge_table[i64.u16 b, i64.u16 a]
        let states =
            input
            |> map i64.u8
            |> map (\x -> initial_state[x])
            |> scan merge g.identity_state
        let produces_token = map (\x -> (x & produces_token_mask) != 0) states
        in
            states
            |> rotate (-1)
            |> map2 (\i x -> if i == 0 then start_state else x) (iota n)
            |> map (\x -> final_state[i64.u16 (x & !produces_token_mask)])
            |> zip produces_token
            |> filter (\(produces_token, _) -> produces_token)
            |> map (\(_, token) -> token)
}
