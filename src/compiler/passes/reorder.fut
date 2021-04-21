import "util"
import "../util"
import "../../../lib/github.com/diku-dk/sorts/radix_sort"

-- Other passes might remove and even reorder some parts of the tree
-- so that its no longer in pre-order. This pass servers to compactify such
-- trees, and really remove any non-active nodes (which point to themselves).

-- | Build an array, with for each node, the index of the previous sibling.
let build_sibling_vector [n] (parents: [n]i32) (depths: [n]i32): [n]i32 =
    -- Assume that at this point, there are no invalid subtrees anymore (as removed by compactify)
    let max_depth = i32.maximum depths
    -- Compute an ordering for nodes according to their depth.
    let order =
        iota n
        |> map i32.i64
        |> radix_sort
            (bit_width max_depth)
            (\bit index -> i32.get_bit bit depths[index])
    -- Sort the parents array by (depth_ order.
    -- Note: These are still the original parents of course, and _not_ the parent into the sorted nodes array!
    let parents_ordered = gather parents order
    -- Compute a mask which is true when a node is the first sibling of its parent, which can be done
    -- simply by checking whether its parent and the parent of the previous node are the same.
    -- Note: This works because the radix sort is stable, and because child nodes of some parent node are
    -- supposed to be laid out in memory such that left siblings have a lower index.
    -- Note: Explicitly set the root nodes to be a first child, this will help further down the pipe.
    let is_first_child_ordered = tabulate n (\i -> i == 0 || parents_ordered[i] != parents_ordered[i - 1])
    -- Compute the ordered sibling vector according to the order and the is_first_child mask.
    -- - If a node is the first child, its sibling should be set to -1.
    -- - Else, use the order vector to obtain the previous sibling.
    let siblings_ordered =
        tabulate
            n
            (\i ->
                if is_first_child_ordered[i] then -1i32
                else order[i - 1])
    -- Finally, un-sort this array to obtain the final sibling vector.
    in
        scatter
            (replicate n (-1i32))
            (map i64.i32 order)
            siblings_ordered

-- | This function computes for each node a pointer to its right-most leaf node.
-- The right most leaf of a leaf node is itself.
let build_right_leaf_vector [n] (parents: [n]i32) (prev_siblings: [n]i32): [n]i32 =
    -- First, compute whether this is the last child by scattering (inverting) the prev sibling array.
    scatter
        (replicate n true)
        (map i64.i32 prev_siblings)
        (replicate n false)
    -- Compute a 'last child' vector, by scattering a node's index to the parent _if_ its the last child.
    |> map2 (\parent is_last_child -> if is_last_child then parent else -1) parents
    |> invert
    -- Now, to find the right most leaf, simply compute for each node a pointer to its root.
    |> find_roots

-- | This function builds a preorder ordering, returning the new parents array and a mapping of new indices to old
-- indices.
let build_preorder_ordering [n] (parents: [n]i32) (prev_siblings: [n]i32) (right_leafs: [n]i32): ([n]i32, [n]i32) =
    let new_index =
        prev_siblings
        -- Compute the pre-order vector, which for every node indicates the next node in the pre-ordering.
        |> map2
            (\parent prev_sibling ->
                if prev_sibling == -1 then parent
                else right_leafs[prev_sibling])
            parents
        -- Now, to compute the new index of each node, simple compute its depth in this pre-ordering parent vector.
        |> compute_depths
    -- Invert to gain an array which for each node in the new array gives the position of the node in the old array.
    let old_index = invert new_index
    -- Compute the new parents array simply by looking up the new position for each parent.
    let new_parents =
        old_index
        |> gather parents
        |> map (\i -> if i == -1 then -1 else new_index[i])
    in (new_parents, old_index)

-- | This function computes for each node a pointer to its right-most leaf node.
-- The right most leaf of a leaf node is itself.
-- This function is basically the same as `build_right_leaf_vector`, but Marcel fucked up the defintiions of pre- and post order.
let build_left_leaf_vector [n] (parents: [n]i32) (prev_siblings: [n]i32): [n]i32 =
    prev_siblings
    -- First, compute a mask of whether this child is the first of its parent.
    |> map (== -1)
    -- Compute a 'first child' vector, by scattering a node's index to the parent _if_ its the first child.
    |> map2 (\parent is_first_child -> if is_first_child then parent else -1) parents
    |> invert
    -- Now, to find the left most leaf, simply compute for each node a pointer to its root.
    |> find_roots

-- | This function builds a postorder ordering, returning the new parents array and a mapping of new indices to old
-- indices.
-- This function is basically the same as `build_preorder_ordering`, but Marcel fucked up the defintions of pre- and post order.
let build_postorder_ordering [n] (parents: [n]i32) (prev_siblings: [n]i32) (left_leafs: [n]i32): ([n]i32, [n]i32) =
    let new_index =
        -- First, invert the prev_siblings array so that we get a next sibling array.
        invert prev_siblings
        -- Compute the (inverted) post order vector, which for every node indicates the next node in the post-ordering.
        |> map2
            (\parent next_sibling ->
                if next_sibling == -1 then parent
                else left_leafs[next_sibling])
            parents
        -- Invert the ordering to obtain the final order
        |> invert
        -- Now, to compute the new index of each node, simplu compute it's depth in this post-ordering parent vector.
        |> compute_depths
    -- Invert to gain an array which for each node in the new array gives the position of the node in the old array.
    let old_index = invert new_index
    -- Compute the new parents array simply by looking up the new position for each parent.
    let new_parents =
        old_index
        |> gather parents
        |> map (\i -> if i == -1 then -1 else new_index[i])
    in (new_parents, old_index)
