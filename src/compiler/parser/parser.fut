import "bracket_matching"
import "../util"
module string = import "../string"
module bt = import "binary_tree"

-- This file should be kept in sync with src/lpg/parser/render.cpp

type~ strtab [n] [m] 't = {
    table: [n]t,
    refs: [m][m](i32, i32)
}

let mk_strtab [n] [m] 't (table: [n]t) (offsets: [m][m]i32) (lengths: [m][m]i32): strtab [n] [m] t =
    {
        table = table,
        refs = map2 zip offsets lengths
    }

module type grammar = {
    module production: integral
    val num_productions: i64

    module token: integral
    val special_token_soi: token.t
    val special_token_eoi: token.t
    val num_tokens: i64

    module bracket: integral
}

module parser (g: grammar) = {
    type~ stack_change_table [n] = strtab [n] [g.num_tokens] g.bracket.t
    type~ parse_table [n] = strtab [n] [g.num_tokens] g.production.t
    type~ arity_array = [g.num_productions]i32

    let is_open_bracket (b: g.bracket.t) =
        (g.bracket.to_i64 b) % 2 == 1

    let is_bracket_pair (a: g.bracket.t) (b: g.bracket.t) =
        (g.bracket.to_i64 a) - (g.bracket.to_i64 b) == 1

    let check [n] [m] (input: [n]g.token.t) (sct: stack_change_table [m]): bool =
        -- Evaluate the RBR/LBR functions for each pair of input tokens
        -- RBR(a) and LBR(w^R) are pre-concatenated by the parser generator
        let (offsets, lens) =
            iota (n + 1)
            |> map (\i ->
                let x = if i == 0 then g.special_token_soi else input[i - 1]
                let y = if i == n then g.special_token_eoi else input[i]
                in copy sct.refs[g.token.to_i64 x, g.token.to_i64 y])
            |> unzip
        -- Check whether all the values are valid (not -1)
        let bracket_refs_valid = offsets |> all (>= 0)
        -- Early return if there is an error
        in if !bracket_refs_valid then false else
        -- Extract the stack changes from the grammar
        string.extract
            sct.table
            offsets
            lens
        -- Check whether the stack changes match up
        |> check_brackets_bt -- or use check_brackets_radix
            is_open_bracket
            is_bracket_pair

    -- Input is expected to be `check`ed at this point. If its not valid according to `check`,
    -- this function might produce invalid results.
    let parse [n] [m] (input: [n]g.token.t) (pt: parse_table [m]): []g.production.t =
        let (offsets, lens) =
            iota (n + 1)
            |> map (\i ->
                let x = if i == 0 then g.special_token_soi else input[i - 1]
                let y = if i == n then g.special_token_eoi else input[i]
                in copy pt.refs[g.token.to_i64 x, g.token.to_i64 y])
            |> unzip
        in string.extract
            pt.table
            offsets
            lens

    -- Given a parse, as generated by the `parse` function, build a parent vector. For each
    -- production in the parse, the related index in the parent vector points to the production
    -- which produced it.
    let build_parent_vector [n] (parse: [n]g.production.t) (arities: arity_array): [n]i32 =
        let tree =
            parse
            -- Get the arity (the number of nonterminals in its RHS; its number of children
            -- in the parse tree) of each production.
            |> map (\p -> arities[g.production.to_i64 p])
            -- Map it to a stack change: Every production would pop itself (1 value) and push
            -- its children (number of children). Thus, final stack change is #children - 1.
            |> map (+ -1)
            -- Calculate the depth
            |> exclusive_scan (+) 0
            -- We are going to find the parent of each node using a previous-smaller-or-equal
            -- scan, which requires a binary tree. Build the binary tree.
            |> bt.construct i32.min i32.highest
        -- For each node, look up its parent by finding the index of the previous
        -- smaller or equal depth.
        in iota n
        |> map i32.i64
        |> map (bt.find_psev tree)
}
