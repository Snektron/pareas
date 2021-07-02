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

-- | Checks whether the input matches the lexer, without returning tokens
let check [n] [m] 'token (input: [n]u8) (table: lex_table [m] token): bool =
    let merge (a: state) (b: state) =
        let a = a & !produces_token_mask
        let b = b & !produces_token_mask
        in table.merge_table[state.to_i64 a, state.to_i64 b]
    let final_state =
        input
        |> map (\x -> table.initial_state[u8.to_i64 x])
        |> reduce merge table.identity_state
    -- Input matches the lexer if the final state generates a token
    in (final_state & produces_token_mask) != 0

-- | Lex the input according to the lexer defined by lex_table.
-- This function returns an array of (token, start-offset, length).
let lex [n] [m] 'token (input: [n]u8) (table: lex_table [m] token): [](token, i32, i32) =
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
    -- Produce a mask for each state specifying whether it's going to be a token.
    let produces_token =
        states
        -- Check whether this transition produced a token.
        |> map (\x -> (x & produces_token_mask) != 0)
        -- If a transition produced a token, the token in question is given by the state that is moved away
        -- from. Shift the produces token array to line them up. This has a double effect: When the state
        -- machine ends in an invalid state, this will produce the invalid token. This is why `true` is shifted
        -- into the right end.
        |> shift_left true
    -- Calculate the indices of states which are going to produce a token.
    let is =
        indices states
        |> map i32.i64
        |> zip produces_token
        |> filter (\(p, _) -> p)
        |> map (\(_, i) -> i)
    -- Calculate the end indices of each token
    let ends = is |> map (+1)
    -- Calculate the start indices by shifting in zero
    let starts = ends |> shift_right 0
    -- Calculate the lengths from the differences
    let lens = map2 (-) ends starts
    -- Finally, compute the actual tokens by performing two gathers.
    let tokens =
        is
        |> map (\i -> states[i])
        |> map (\s -> table.final_state[state.to_i64 (s & !produces_token_mask)])
    in zip3 tokens starts lens
