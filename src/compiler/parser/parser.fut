import "string_packing"

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
    type production = g.production
    type token = g.token

    module bracket = g.bracket

    -- For now expected to include soi and eoi
    let check [n] (input: [n]g.token) =
        -- Evaluate the RBR/LBR functions for each pair of input tokens
        -- RBR(a) and LBR(w^R) are pre-concatenated by the parser generator, and
        -- the initial stack change is generated separately
        let (bracket_offsets, bracket_lens) =
            map3
                (\a b i -> if i == 0
                    then g.initial_stack_change
                    else g.get_stack_change a b)
                (rotate (-1) input)
                input
                (iota n)
            |> unzip
        let brackets =
            pack_nonempty_strings
                g.stack_change_table
                (map (g.stack_change_offset.to_i64) bracket_offsets)
                (map (g.stack_change_offset.to_i64) bracket_lens)
        in brackets
}
