import "string_packing"
import "bracket_matching"

module type grammar = {
    type token
    type production

    module bracket: integral
    module stack_change_offset: integral
    val stack_change_table_size: i64
    val stack_change_table: [stack_change_table_size]bracket.t
    val initial_stack_change: (stack_change_offset.t, stack_change_offset.t)
    val get_stack_change: (a: token) -> (b: token) -> (stack_change_offset.t, stack_change_offset.t)

    module parse_offset: integral
    val parse_table_size: i64
    val parse_table: [parse_table_size]production
    val initial_parse: (parse_offset.t, parse_offset.t)
    val get_parse: (a: token) -> (b: token) -> (parse_offset.t, parse_offset.t)
}

module parser (g: grammar) = {
    let is_open_bracket (b: g.bracket.t) =
        (g.bracket.to_i64 b) % 2 == 1

    let is_bracket_pair (a: g.bracket.t) (b: g.bracket.t) =
        (g.bracket.to_i64 a) - (g.bracket.to_i64 b) == 1

    -- For now expected to include soi and eoi
    let check [n] (input: [n]g.token) =
        -- Evaluate the RBR/LBR functions for each pair of input tokens
        -- RBR(a) and LBR(w^R) are pre-concatenated by the parser generator, and
        -- the initial stack change is generated separately
        let (offsets, lens) =
            map3
                (\a b i -> if i == 0
                    then g.initial_stack_change
                    else g.get_stack_change a b)
                (rotate (-1) input)
                input
                (iota n)
            |> unzip
        -- Check whether all the values are valid (not -1)
        let bracket_refs_valid = offsets |> map g.stack_change_offset.to_i64 |> all (>= 0)
        -- Early return if there is an error
        in if !bracket_refs_valid then false else
        -- Extract the stack changes from the grammar
        let brackets =
            pack_nonempty_strings
                g.stack_change_table
                (map (g.stack_change_offset.to_i64) offsets)
                (map (g.stack_change_offset.to_i64) lens)
        -- Early return if the amount of brackets isn't even
        in if (length brackets) % 2 != 0 then false else
        -- Build the bracket match array
        -- TODO: If we only aim to check whether brackets match below function
        -- could probably be optimized
        let bmatch = argpair_brackets (map is_open_bracket brackets)
        -- Finally, check if brackets match up
        in all
            (\(i, j) -> is_bracket_pair brackets[i64.u32 i] brackets[i64.u32 j])
            bmatch

    -- For now expected to include soi and eoi
    -- Input is expected to be `check`ed at this point. If its not valid according to `check`,
    -- this function might crash!
    let parse [n] (input: [n]g.token) =
        let (offsets, lens) =
            map3
                (\a b i -> if i == 0
                    then g.initial_parse
                    else g.get_parse a b)
                (rotate (-1) input)
                input
                (iota n)
            |> unzip
        in pack_strings
            g.parse_table
            (map (g.parse_offset.to_i64) offsets)
            (map (g.parse_offset.to_i64) lens)
}
