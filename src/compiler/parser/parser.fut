import "string_packing"
import "bracket_matching"
import "../util"

module type grammar = {
    module production: integral
    module token: integral

    val num_tokens: i64

    module bracket: integral
    module stack_change_offset: integral
    val stack_change_table_size: i64
    val stack_change_table: [stack_change_table_size]bracket.t
    val stack_change_refs: [num_tokens][num_tokens](stack_change_offset.t, stack_change_offset.t)

    module parse_offset: integral
    val parse_table_size: i64
    val parse_table: [parse_table_size]production.t
    val parse_refs: [num_tokens][num_tokens](parse_offset.t, parse_offset.t)
}

module parser (g: grammar) = {
    let is_open_bracket (b: g.bracket.t) =
        (g.bracket.to_i64 b) % 2 == 1

    let is_bracket_pair (a: g.bracket.t) (b: g.bracket.t) =
        (g.bracket.to_i64 a) - (g.bracket.to_i64 b) == 1

    -- For now expected to include soi and eoi
    let check [n] (input: [n]g.token.t) =
        -- Evaluate the RBR/LBR functions for each pair of input tokens
        -- RBR(a) and LBR(w^R) are pre-concatenated by the parser generator
        let (offsets, lens) =
            in_windows_of_pairs input
            |> map (\(a, b) -> copy g.stack_change_refs[g.token.to_i64 a, g.token.to_i64 b])
            |> unzip
        -- Check whether all the values are valid (not -1)
        let bracket_refs_valid = offsets |> map g.stack_change_offset.to_i64 |> all (>= 0)
        -- Early return if there is an error
        in if !bracket_refs_valid then false else
        -- Extract the stack changes from the grammar
        pack_nonempty_strings
            g.stack_change_table
            (map g.stack_change_offset.to_i64 offsets)
            (map g.stack_change_offset.to_i64 lens)
        -- Check whether the stack changes match up
        |> check_brackets_bt -- or use check_brackets_radix
            is_open_bracket
            is_bracket_pair

    -- For now expected to include soi and eoi
    -- Input is expected to be `check`ed at this point. If its not valid according to `check`,
    -- this function might crash!
    let parse [n] (input: [n]g.token.t) =
        let (offsets, lens) =
            in_windows_of_pairs input
            |> map (\(a, b) -> copy g.parse_refs[g.token.to_i64 a, g.token.to_i64 b])
            |> unzip
        in pack_strings
            g.parse_table
            (map g.parse_offset.to_i64 offsets)
            (map g.parse_offset.to_i64 lens)
}
