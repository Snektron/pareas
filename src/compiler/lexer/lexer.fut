import "../util"

-- This file should be kept in sync with src/lpg/lexer/render.hpp, src/lpg/lexer/parallel_lexer.hpp
-- and src/lpg/lexer/fsa.hpp.

module state = u16
type state = state.t

local let produces_token_mask: state = 0x8000

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

-- | Lex the input according to the lexer defined by lex_table.
let lex [n] [m] 'token (input: [n]u8) (table: lex_table [m] token): []token=
    let merge (a: state) (b: state) =
        let a = a & !produces_token_mask
        let b = b & !produces_token_mask
        in table.merge_table[state.to_i64 a, state.to_i64 b]
    -- Compute the initial states over the input
    let states =
        input
        -- First, compute the initial state for each input character
        |> map (\x -> table.initial_state[u8.to_i64 x])
        -- Perform the actual lexing phase: each pair of states is combined according to the merge table.
        |> scan merge table.identity_state
    let produces_token =
        states
        -- Check whether this transition produced a token.
        |> map (\x -> (x & produces_token_mask) != 0)
        |> shift_left true
    in
        states
        |> map (\x -> table.final_state[state.to_i64 (x & !produces_token_mask)])
        |> zip produces_token
        |> filter (\(produces_token, _) -> produces_token)
        |> map (\(_, token) -> token)
