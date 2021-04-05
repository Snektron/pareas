import "../util"
import "../../../lib/github.com/diku-dk/sorts/radix_sort"

-- Other passes might remove and even reorder some parts of the tree
-- so that its no longer in pre-order. This pass servers to compactify such
-- trees, and really remove any non-active nodes (which point to themselves).

local let compute_depths [n] (parents: [n]i32): [n]i32 =
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

local let right_descendant [n] (parents: [n]i32) (is_last_child: [n]bool): [n]i32 =
    let (_, rd) =
        iterate
            (n |> i32.i64 |> bit_width)
            (\(links, rd) ->
                let is =
                    links
                    |> map (\link-> if link == -1 then -1 else link)
                    |> map i64.i32
                let rd' =
                    scatter
                        (copy rd)
                        is
                        rd
                let links' = map (\link -> if link == -1 || !is_last_child[link] then -1 else links[link]) links
                in (links', rd'))
            (parents, iota n |> map i32.i64)
   in rd

-- | Other passes (for example `fix_bin_ops`@term@"fix_bin_ops") might reorder some nodes, which causes
-- the tree to no longer be in pre-order, which is required for some subsequent passes. This function
-- reorders them back into preorder, returning the new parents array, and an array that can be used to
-- gather any node data into a new array in pre-order.
-- This function requires that there be no invalid nodes, which are to be removed using the
-- `compactify`@term@"compactify" pass.
let make_preorder_ordering [n] (parents: [n]i32): ([n]i32, [n]i32) =
    -- Assume that at this point, there are no invalid subtrees anymore (as removed by compactify)
    let depths = compute_depths parents
    let max_depth = 1 + reduce i32.max (-1) depths
    -- Compute an ordering for nodes according to their depth.
    let order =
        iota n
        |> map i32.i64
        |> radix_sort
            (bit_width max_depth)
            (\bit index -> i32.get_bit bit depths[index])
    -- Sort the parents array by order.
    -- Note: These are still the original parents of course, and _not_ the parent into the sorted nodes array!
    let sorted_parents = map (\i -> parents[i]) order
    -- Compute a mask which is true when a node is the first sibling of its parent, which can be done
    -- simply by checking whether its parent and the parent of the previous node are the same.
    -- Note: This works because the radix sort is stable, and because child nodes of some parent node are
    -- supposed to be laid out in memory such that left siblings have a lower index.
    -- Note: Explicitly set the root nodes to be a first child, this will help further down the pipe.
    let sorted_is_first_child =
        iota n
        |> map (\i -> i == 0 || sorted_parents[i] != sorted_parents[i - 1])
    -- Since we explicitly set the root node as being the first child, the last node will also be set as last child.
    let sorted_is_last_child = rotate 1 sorted_is_first_child
    -- The sorted_is_last_child array is in order of the nodes sorted by depth, however,the rightmost descendant
    -- computation is easier to perform on the unsorted array, as it requires us to follow parent pointers.
    -- These would otherwise need to be recomputed for the sorted array, which requires 2 scatters.
    let is_last_child =
        scatter
            (replicate n false)
            (map i64.i32 order)
            sorted_is_last_child
    let rd = right_descendant parents is_last_child
    let sorted_prev_rd =
        map (\i -> rd[i]) order
        -- Rotate the sorted right descendants one to the right so that for every non-first-child node it contains
        -- the right descendant the previous child.
        |> rotate (-1)
    let sorted_post_order =
        map3 (\first_child prev_rd parent ->
            -- If this is the first (or only) child of a node, then the next node in
            -- is the parent node.
            -- Note: Because we explicitly set the first node as being the first child, the chain will
            -- end there, as the parent of the root node is of course -1.
            if first_child then parent
            -- Else, the next node is the right descendant of the left sibling's right descendant
            else prev_rd)
        sorted_is_first_child
        sorted_prev_rd
        sorted_parents
    -- Un-sort the generated post order to obtain, for each node, a pointer to the previous node in post-order.
    -- This is required because we computed the sorted_post_order using the original parents instead of parents
    -- for the sorted array.
    let post_order =
        scatter
            (replicate n 0i32)
            (map i64.i32 order)
            sorted_post_order
    -- To obtain the new index, simply compute the depth of nodes according to this post order.
    let new_index = compute_depths post_order
    -- Compute the inverted array of these indices.
    let old_index =
        scatter
            (replicate n 0i32)
            (map i64.i32 new_index)
            (iota n |> map i32.i64)
    -- Compute the new parents array.
    let parents =
        old_index
        |> gather parents
        |> map (\i -> if i == -1 then -1 else new_index[i])
    in (parents, old_index)
