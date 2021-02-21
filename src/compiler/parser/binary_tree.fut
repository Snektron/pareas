import "../util"

-- TODO: This module only handles *full* binary trees, and thus wastes some memory
-- Additionally, the leaves and internal nodes might be also stored separately to
-- improve memory usage.

-- Utility function to write a value raised to the power of 2 nicer.
local let pow2 (x: i32) = 1 << x

-- Compute the hight a binary tree would have, given the number of leaves.
-- The number of leaves is rounded up to the nearest power of 2.
let height_from_leaves (num_leaves: i32): i32 = bit_width (num_leaves - 1)

-- Compute the hight a binary tree would have, given the total number of nodes.
-- (internal and leaves) in the tree.
let height_from_tree (tree_size: i32): i32 = (bit_width tree_size) - 1

-- Compute the number of elements an array backing a binary tree requires.
-- given the hight of the tree
let array_size (height: i32): i64 = i64.i32 (pow2 (height + 1) - 1)

-- Compute the element offset in the backing storage where a certain level starts.
let level_offset (level: i32): i32 = pow2 level - 1

-- Compute the number of nodes in a certain level.
let level_size (level: i32): i32 = pow2 level

-- Given the index of some node, compute its parent index. The child's index
-- must not be the root node.
let parent (i: i32) = (i - 1) / 2

-- Given the index of some node, compute the index of its left child.
let left (i: i32) = 2 * i + 1

-- Given the index of some node, compute the index of its right child.
let right (i: i32) = 2 * i + 2

-- Given the index of some node, compute the index of its sibling.
let sibling (i: i32) = if i % 2 == 0 then i - 1 else i + 1

-- Given the index of some node, compute whether this node is the left or right
-- child of its parent.
let is_left (i: i32) = i % 2 == 1

-- Given an array of leaves, construct a binary tree. The internal leaves are
-- computed by applying `op` on their children. If the array of leaves `xs` does not
-- fill all the leaves in the (complete) binary tree, it is padded with `ne`.
let construct [n] (op: i32 -> i32 -> i32) (ne: i32) (xs: [n]i32): []i32 =
    let h = height_from_leaves (i32.i64 n)
    -- Compute the initial tree by scattering the leaves to the appropriate location
    -- in the backing array.
    let init =
        scatter
            (replicate (array_size h) ne)
            (tabulate n (+i64.i32 (level_offset h)))
            xs
    let (_, tree) =
        -- Compute the binary tree by looping over levels until we reach the root node.
        loop (level, tree) = (h, init) while level > 0 do
            -- Compute offsets and sizes of the relevant arrays.
            let children_offset = level_offset level |> i64.i32
            let children_size = level_size level |> i64.i32
            let parent_offset = level_offset (level - 1) |> i64.i32
            let parent_size = level_size (level - 1) |> i64.i32
            -- Compute the parent values from the children.
            let parents =
                -- This copy is required as `parents` would otherwise alias with
                -- `tree` in the scatter operation below.
                copy tree[children_offset : children_offset + children_size]
                |> in_pairs
                |> map (\(a, b) -> a `op` b)
            -- Scatter the parent values to the appropriate location in the tree
            let tree =
                scatter
                    tree
                    (tabulate parent_size (+parent_offset))
                    (parents :> [parent_size]i32)
            in (level - 1, tree)
    in tree

-- Generic function to find the a previous value according to some relational operator.
-- If no such value exists, returns -1.
-- This function is intended for (<) and (<=) in other functions.
local let find_pv [n] (op: i32 -> i32 -> bool) (tree: [n]i32) (leaf: i32): i32 =
    let h = height_from_tree (i32.i64 n)
    -- Compute the offset of the leaves within the tree
    let base = level_offset h
    -- Compute the absolute index of the leaf
    let start = leaf + base
    let value = tree[start]
    -- Go up the tree to find the right child of the common ancestor.
    -- The common ancestor is the first node up the tree which right child has
    -- a value smaller than the value of the leaf.
    --
    --       [0]       -- The common ancestor of 1 and 0 is 0 in the root.
    --    0      [1]   -- The right child of the common ancestor is 1.
    --  0   5   1   2
    -- 3[0]5 6[1]4 7 2 -- start at 1, the target is 0.
    let index =
        iterate_while
            (\i -> i != 0 && (is_left i || !(tree[sibling i] `op` value)))
            parent
            start
    -- If we reach the root of the tree, there is no match. Early return in that case.
    in if index == 0 then -1 else
    -- Compute the final match by going down the tree again.
    -- The right child is preferred, but only traversed if it is smaller than
    -- the value which we're looking for.
    let index =
        iterate_while
            -- Iterate while the index is not a leaf
            (< base)
            (\i -> if tree[right i] `op` value then right i else left i)
            -- Start at the right child of the common ancestor - the sibling of the
            -- left child.
            (sibling index)
    -- The index is absolute, so compute the leaf-index.
    in index - base

-- Given a binary tree and a leaf-index, find the leaf-index of the previous
-- value smaller than the value of the leaf. If no such value is present, this
-- function returns -1.
let find_psv [n] (tree: [n]i32) (leaf: i32): i32 =
    find_pv (<) tree leaf

-- Given a binary tree and a leaf-index, find the leaf-index of the previous
-- value smaller than or equal to the value of the leaf. If no such value is present,
-- this function returns -1.
let find_psev [n] (tree: [n]i32) (leaf: i32): i32 =
    find_pv (<=) tree leaf
