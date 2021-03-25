import "parser/parser"
module lexer = import "lexer/lexer"
module g = import "../../gen/pareas_grammar"
module pareas_parser = parser g

type production = g.production.t

let list_end_productions = [
    g.production_logical_or_end,
    g.production_logical_and_end,
    g.production_rela_end,
    g.production_bitwise_end,
    g.production_shift_end,
    g.production_sum_end,
    g.production_prod_end
]

-- Lists have the form
--    sum
--   / \
--  A   sum_add
--     / \
--    B   sum_add
--       / \
--      C   sum_end
-- We are going to remove the ends by transforming the lists into
--    sum_add
--   / \
--  A   sum_add
--     / \
--    C   D
-- This removes redundant list end nodes (marked as invalid after).
-- This operation has a double purpose: consider the tree
--       expr
--       |
--       sum
--      / \
--  prod   end
--    / \
--   id  end
-- This step will clean up the unused prod and sum nodes as well.
let clean_up_lists [n] (tree: [n]production) (parents: [n]i32) =
    -- First, generate a marking for each node whether it should be removed in this step.
    -- Generate the initial marking simply by checking whether the node is part
    -- of the predefined list of end nodes.
    let ends = map (\node -> any (== node) list_end_productions) tree
    -- Generate a new marking of nodes containing a child which is a list end. This can
    -- be done simply by scattering the list end marking one edge up the parent tree.
    -- We can simply re-use the existing array for this.
    -- Also note that in theory, each node may only have a maximum of one possible list end
    -- child, so we can simply use a scatter here without conflicts.
    -- We only want to scatter some values though (those with `ends[i]` set to true), and
    -- for the others set -1 as index, which Futhark will ignore.
    let is = map2 (\parent is_end -> if is_end then parent else -1) parents ends
    -- Scatter the markings to the ends array.
    let ends =
        scatter
            ends
            (map i64.i32 is)
            (replicate n true)
    -- Next, find the new parent array. For each node, walk up the tree as long as the parent contains
    -- a marked node.
    -- TODO: This could maybe be improved using a prefix-sum like approach?
    let find_new_parent (node: i32): i32 =
        loop node = node while ends[parents[node]] do
            parents[node]
    in
        iota n
        |> map i32.i64
        |> zip ends
        |> map (\(is_end, node) ->
            if is_end
            then -1 -- mark this node as 'to be removed'
            else find_new_parent node)

let main [n] [m]
    (input: [n]u8)
    (initial_state: [256]lexer.state)
    (merge_table: [m][m]lexer.state)
    (final_state: [m]g.token.t) =
    let lt =
        lexer.mk_lex_table
            initial_state
            merge_table
            final_state
            g.identity_state
    let (tokens, _, _) =
        lexer.lex input lt
        |> filter (\(t, _, _) -> t != g.token_whitespace && t != g.token_comment)
        |> unzip3
    -- As the lexer returns an `invalid` token when the input cannot be lexed, which is accepted
    -- by the parser also, pareas_parser.check will fail whenever there is a lexing error.
    in if !(pareas_parser.check tokens) then -1 else
    let tree = pareas_parser.parse tokens
    let parents = pareas_parser.build_parent_vector tree
    let parents = clean_up_lists tree parents
    in last parents
