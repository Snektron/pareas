type state = u16

-- Keep in sync with render.hpp
local let produces_token_mask: state = 0x8000

-- Keep in sync with parallel_lexer.hpp and fsa.hpp
local let reject_state: state = 0
local let start_state: state = 1

type~ lex_table [m] 'token = {
    initial_state: [256]state,
    merge_table: [m][m]state,
    final_state: [m]token,
    identity_state: state
}

let lex [n] [m] 'token (input: [n]u8) (table: lex_table [m] token): []token =
    let merge (a: state) (b: state) =
        let a = a & !produces_token_mask
        let b = b & !produces_token_mask
        in table.merge_table[i64.u16 b, i64.u16 a]
    let states =
        input
        |> map i64.u8
        |> map (\x -> table.initial_state[x])
        |> scan merge table.identity_state
    let produces_token = map (\x -> (x & produces_token_mask) != 0) states
    in
        states
        |> rotate (-1)
        |> map2 (\i x -> if i == 0 then start_state else x) (iota n)
        |> map (\x -> table.final_state[i64.u16 (x & !produces_token_mask)])
        |> zip produces_token
        |> filter (\(produces_token, _) -> produces_token)
        |> map (\(_, token) -> token)
