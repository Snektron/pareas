import "../util"

-- | The parser generates quite some superficial nodes, which can slow the application of other passes
-- down. These are marked by the `fix_bin_ops`@term@"fix_bin_ops" pass, but not actually removed. Futhermore, the final
-- output is supposed to be in pre-order, with no invalid nodes.
-- This pass removes all those invalid nodes, returning the new parents array, and an array
-- that can be used to gather any node data into a new, compactified array.
-- This function returns an array of (parent, old_index), which should be unzipped, so that the futhark
-- compiler can prove that these arrays are of the same length.
let compactify [n] (parents: [n]i32): [](i32, i32) =
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
    in zip parents old_index
