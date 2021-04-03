import "util"
import "../util"
import "../../../gen/pareas_grammar"
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
    -- Fix up depths of invalid nodes and depths being started at one.
    let depths =
        map3
            (\i depth parent ->
                if i == parent then -1 else depth - 1)
            (iota n |> map i32.i64)
            depths
            parents
    in depths

local let right_grandchild [n] (parents: [n]i32) (is_last_child: [n]bool): [n]i32 =
    let (_, rgc) =
        iterate
            (n |> i32.i64 |> bit_width)
            (\(links, rgc) ->
                let is =
                    links
                    |> map (\link-> if link == -1 then -1 else link)
                    |> map i64.i32
                let rgc' =
                    scatter
                        (copy rgc)
                        is
                        rgc
                let links' = map (\link -> if link == -1 || !is_last_child[link] then -1 else links[link]) links
                in (links', rgc'))
            (parents, iota n |> map i32.i64)
   in rgc

let compactify [n] (parents: [n]i32) =
    -- TODO: Mark all nodes of deleted subtrees as deleted by setting their parents to themselves.
    -- Make a mask specifying whether a node should be included in the new tree.
    let include_mask =
        iota n
        |> map i32.i64
        |> zip parents
        |> map (\(i, parent) -> parent != i)
    let is =
        include_mask
        |> map i32.bool
        |> scan (+) 0
    -- break up the computation of is temporarily to get the size of the new arrays.
    let m = last is |> i64.i32
    -- For a node index i in the old array, this array gives the position in the new array (which should be of size m)
    let new_index =
        map2 (\inc i -> if inc then i else -1) include_mask is
        |> map (+ -1)
    -- For a node index j in the new array, this gives the position in the old array
    let old_index =
        scatter
            (replicate m 0i32)
            (new_index |> map i64.i32)
            (iota n |> map i32.i64)
    -- Also compute the new parents array here, since we need the `is` array for it, but dont need it anywhere else.
    let parents =
        -- Begin with the indices into the old array
        old_index
        -- Gather its parent, which points to an index into the old array as well
        |> gather parents
        -- Find the index into the new array
        |> map (\i -> if i == -1 then -1 else new_index[i])
    in (parents, old_index)

let make_preorder_ordering [n] (parents: [n]i32): [n]i32 =
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
    -- The sorted_is_last_child array is in order of the nodes sorted by depth, however,the rightmost grandchild
    -- computation is easier to perform on the unsorted array, as it requires us to follow parent pointers.
    -- These would otherwise need to be recomputed for the sorted array, which requires 2 scatters.
    let is_last_child =
        scatter
            (replicate n false)
            (map i64.i32 order)
            sorted_is_last_child
    let rgc = right_grandchild parents is_last_child
    let sorted_prev_rgc =
        map (\i -> rgc[i]) order
        -- Rotate the sorted grand childen one to the right so that for every non-first-child node it contains
        -- the right grandchild of the previous child.
        |> rotate (-1)
    let sorted_post_order =
        map3 (\first_child prev_rgc parent ->
            -- If this is the first (or only) child of a node, then the next node in
            -- is the parent node.
            -- Note: Because we explicitly set the first node as being the first child, the chain will
            -- end there, as the parent of the root node is of course -1.
            if first_child then parent
            -- Else, the next node is the right grand child of the left sibling's right grand child
            else prev_rgc)
        sorted_is_first_child
        sorted_prev_rgc
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
    in new_index
