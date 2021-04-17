import "../util"
import "../../../gen/pareas_grammar"

-- | Given a tree and a marking for each node, computes the first ancestor node which is unmarked.
-- If the root node is also marked, the new parent is the root node.
-- Returns a new list of parents for each node.
-- Note: This implementation is linear in parallel time, and should be used where
-- a small amount of subsequent nodes that need to be removed is expected.
-- For an implementation that is more efficient when a large amount of subsequent nodes needs
-- to be removed, see `find_unmarked_parents_log`@term.
let find_unmarked_parents_lin [n] (parents: [n]i32) (marks: [n]bool): [n]i32 =
    let find_new_parent (node: i32): i32 =
        loop current = parents[node] while current != -1 && parents[current] != current && marks[current] do
            parents[current]
    in
        iota n
        |> map i32.i64
        |> map find_new_parent

-- | Removes marked nodes by adjusting parent pointers of other nodes
-- The parents of the removed nodes are set to their own ID, creating a loop.
-- Remember, the root node is given by a node which' parent is -1.
-- Returns a new list of parents for each node.
-- Note: This implementation is linear in parallel time, and should be used where
-- a small amount of subsequent nodes that need to be removed is expected.
-- For an implementation that is more efficient when a large amount of subsequent nodes needs
-- to be removed, see `remove_nodes_log`@term
let remove_nodes_lin [n] (parents: [n]i32) (remove: [n]bool): [n]i32 =
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

-- | Given a tree and a marking for each node, computes the first ancestor node which is unmarked.
-- If the root node is also marked, the new parent is the root node.
-- Returns a new list of parents for each node.
-- Note: This implementation is logarithmic in parallel time, but has an overhead when only a small amount of
-- subsequent parents need to be removed. For an implementation more efficient in that case, see
-- `find_unmarked_parents_lin`@term.
let find_unmarked_parents_log [n] (parents: [n]i32) (marks: [n]bool): [n]i32 =
    iterate
        (n |> i32.i64 |> bit_width)
        (\links ->
            map
                (\link -> if link == -1 || !marks[link] then link else links[link])
                links)
        parents

-- | Removes marked nodes by adjusting parent pointers of other nodes
-- The parents of the removed nodes are set to their own ID, creating a loop.
-- Remember, the root node is given by a node which' parent is -1.
-- Returns a new list of parents for each node.
-- Note: This implementation is logarithmic in parallel time, but has an overhead when only a small amount of
-- subsequent parents need to be removed. For an implementation more efficient in that case, see
-- `remove_nodes_lin`@term.
let remove_nodes_log [n] (parents: [n]i32) (remove: [n]bool): [n]i32 =
    find_unmarked_parents_log parents remove
    |> map3
        (\i remove parent -> if remove then i else parent)
        (iota n |> map i32.i64)
        remove

-- | Given a tree, compute for each node the zero-based depth of the node.
-- This function runs logarithmic parallel time, but does have a quite large overhead
-- if the tree is very flat.
-- Note: This implementation is logarithmic in parallel time, but has an overhead when small iteration counts
-- are expected, such as when there are multiple small trees.
let compute_depths [n] (parents: [n]i32): [n]i32 =
    let (_, depths) =
        iterate
            (n |> i32.i64 |> bit_width)
            (\(links, depths) ->
                let depths' =
                    links
                    |> map (\link -> if link == -1 then 0 else depths[link])
                    |> map2 (+) depths
                    |> map2 i32.max depths
                let links' = map (\link -> if link == -1 then link else links[link]) links
                in (links', depths'))
            (parents, replicate n 1i32)
    -- Because we start with an array of all ones, the root node will have depth 1.
    in map (+ -1) depths

-- | Given a tree, find for each node the root node.
-- Note: This implementation is logarithmic in parallel time, but has an overhead when only a small amount of
-- iterations is expected.
let find_roots [n] (parents: [n]i32): [n]i32 =
    iterate
        (n |> i32.i64 |> bit_width)
        (\links ->
            map
                (\link -> if link == -1 || links[link] == -1 then link else links[link])
                links)
        parents
    -- Adjust for if the initial node was the root.
    |> map2 (\i p -> if p == -1 then i32.i64 i else p) (iota n)

-- | Make a mask-array of a set of productions
let mk_production_mask [n] (productions: [n]production.t): [num_productions]bool =
    scatter
        (replicate num_productions false)
        (productions |> map i64.u8)
        (productions |> map (\_ -> true))

-- | Make an associative array of production to value
let mk_production_array [n] 't (default: t) (items: [n](production.t, t)): [num_productions]t =
    let (keys, values) = unzip items
    in scatter
        (replicate num_productions default)
        (keys |> map i64.u8)
        values
