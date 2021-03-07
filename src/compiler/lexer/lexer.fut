module state = u16
type state = state.t

-- Keep in sync with render.hpp
local let produces_token_mask: state = 0x8000

-- Keep in sync with parallel_lexer.hpp and fsa.hpp
local let reject_state: state = 0
local let start_state: state = 1

type~ lex_table [n] 'token = {
    initial_state: [256]state,
    merge_table: [n][n]state,
    final_state: [n]token,
    identity_state: state
}

let mk_lex_table [n] 'token
        (initial_state: [256]state)
        (merge_table: [n][n]state)
        (final_state: [n]token)
        (identity_state: state) : lex_table [n] token =
    {
        initial_state = initial_state,
        merge_table = merge_table,
        final_state = final_state,
        identity_state = identity_state
    }

let lex [n] [m] 'token (input: [n]u8) (table: lex_table [m] token): []token =
    let merge (a: state) (b: state) =
        let a = a & !produces_token_mask
        let b = b & !produces_token_mask
        in table.merge_table[state.to_i64 b, state.to_i64 a]
    let states =
        input
        |> map (\x -> table.initial_state[u8.to_i64 x])
        |> scan merge table.identity_state
    let produces_token = map (\x -> (x & produces_token_mask) != 0) states
    in
        states
        |> rotate (-1)
        |> map2 (\i x -> if i == 0 then start_state else x) (iota n)
        |> map (\x -> table.final_state[state.to_i64 (x & !produces_token_mask)])
        |> zip produces_token
        |> filter (\(produces_token, _) -> produces_token)
        |> map (\(_, token) -> token)
