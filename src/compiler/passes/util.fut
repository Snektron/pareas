import "../../../gen/pareas_grammar"

let find_unmarked_parents [n] (parents: [n]i32) (marks: [n]bool): [n]i32 =
    let find_new_parent (node: i32): i32 =
        loop current = parents[node] while current != -1 && parents[current] != current && marks[current] do
            parents[current]
    in
        iota n
        |> map i32.i64
        |> map find_new_parent

-- Removes marked nodes by adjusting parent pointers of other nodes
-- The parents of the removed nodes are set to their own ID, creating a loop.
-- Remember, the root node is given by a node which' parent is -1.
-- Returns a new list of parents for each node.
let remove_nodes [n] (parents: [n]i32) (remove: [n]bool): [n]i32 =
    -- For each node, walk up the tree as long as the parent contains
    -- a marked node.
    -- TODO: This could maybe be improved using a prefix-sum like approach?
    let find_new_parent (node: i32): i32 =
        if remove[node] then node else
        let new_parent = loop current = parents[node] while current != -1 && parents[current] != current && remove[current] do
            parents[current]
        -- To remove the entire subtree at once
        in if new_parent == -1 || !remove[new_parent] then new_parent else node
    in
        iota n
        |> map i32.i64
        |> map find_new_parent

-- Make a mask-array of a set of productions
let mk_production_mask [n] (productions: [n]production.t): [num_productions]bool =
    scatter
        (replicate num_productions false)
        (productions |> map i64.u8)
        (productions |> map (\_ -> true))

-- Make an associative array of production to value
let mk_production_array [n] 't (default: t) (items: [n](production.t, t)): [num_productions]t =
    let (keys, values) = unzip items
    in scatter
        (replicate num_productions default)
        (keys |> map i64.u8)
        values
