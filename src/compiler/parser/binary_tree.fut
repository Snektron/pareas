local let bit_width (x: i32): i32 = i32.num_bits - (i32.clz x)

local let pow2 (x: i32) = 1 << x

local let pairwise_map [n] (op: i32 -> i32 -> i32) (xs: [n]i32): []i32 =
    let m = assert (n % 2 == 0) (n / 2)
    in map2 op (xs[0::2] :> [m]i32) (xs[1::2] :> [m]i32)

let height_from_leaves (num_leaves: i32): i32 = bit_width (num_leaves - 1)

let height_from_tree (tree_size: i32): i32 = (bit_width tree_size) - 1

let array_size (height: i32): i64 = i64.i32 (pow2 (height + 1) - 1)

let level_offset (level: i32): i32 = pow2 level - 1

let level_size (level: i32): i32 = pow2 level

let parent (i: i32) = (i - 1) / 2

let left (i: i32) = 2 * i + 1

let right (i: i32) = 2 * i + 2

let sibling (i: i32) = if i % 2 == 0 then i - 1 else i + 1

let is_left (i: i32) = i % 2 == 1

let construct [n] (op: i32 -> i32 -> i32) (ne: i32) (xs: [n]i32): []i32 =
    let h = height_from_leaves (i32.i64 n)
    let init =
        scatter
            (replicate (array_size h) ne)
            (tabulate n (+i64.i32 (level_offset h)))
            xs
    let (_, tree) =
        loop (level, tree) = (h, init) while level > 0 do
            let children_offset = level_offset level
            let children_size = level_size level
            let parent_offset = level_offset (level - 1)
            let parent_size = i64.i32 (level_size (level - 1))
            let values =
                pairwise_map
                    op
                    (copy tree[i64.i32 children_offset : i64.i32 (children_offset + children_size)])
            let tree =
                scatter
                    tree
                    (tabulate parent_size (+i64.i32 parent_offset))
                    (values :> [parent_size]i32)
            in (level - 1, tree)
    in tree

let find_psv [n] (tree: [n]i32) (leaf: i32): i32 =
    let h = height_from_tree (i32.i64 n)
    let base = level_offset h
    let start = leaf + base
    let value = tree[start]
    -- Go up the tree to find the child of the common ancestor
    let index =
        loop index = start while index != 0 && (is_left index || tree[sibling index] >= value) do
            parent index
    -- Compute ancestor for which to start going down the tree
    in if index == 0 then -1 else
    let start = left (parent index)
    let index =
        loop index = start while index < base do
            if tree[right index] < value
            then right index
            else left index
    in index - base