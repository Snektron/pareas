module g = import "../../../gen/pareas_grammar"

type production = g.production.t

-- Removes marked nodes by adjusting parent pointers of other nodes
-- The parents of the removed nodes are set to their own ID, creating a loop.
-- Remember, the root node is given by a node which' parent is -1.
-- Returns a new list of parents for each node.
let remove_nodes [n] (remove: [n]bool) (parents: [n]i32): [n]i32 =
    -- For each node, walk up the tree as long as the parent contains
    -- a marked node.
    -- TODO: This could maybe be improved using a prefix-sum like approach?
    let find_new_parent (node: i32): i32 =
        if remove[node] then node
        else loop current = parents[node] while current != -1 && parents[current] != current && remove[current] do
            parents[current]
    in
        iota n
        |> map i32.i64
        |> map find_new_parent

-- Make a mask-array of a set of productions
let mk_production_mask [n] (productions: [n]production) =
    scatter
        (replicate g.num_productions false)
        (productions |> map i64.u8)
        (productions |> map (\_ -> true))
