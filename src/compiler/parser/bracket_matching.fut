import "../../../lib/github.com/diku-dk/sorts/radix_sort"

-- Build an array that for every bracket gives its nesting depth. Opening brackets are
-- represented by `true` and closing brackets by `false`.
local let compute_depths [n] (brackets: [n]bool): []i32 =
    brackets
    -- Map each bracket to a stack depth change
    |> map (\b -> if b then 1i32 else -1i32)
    -- Perform prefix sum to build the initial depth vectpr
    |> scan (+) 0
    -- This will result in the right depth for closing parenthesis,
    -- but not for the opening parenthesis:
    -- [ ( ) { ( ) } ]
    -- 1 2 1 2 3 2 1 0 - we have this after the scan
    -- 0 1 1 1 2 2 1 0 - we want to get this
    -- We fix this by simply decreasing the depth of each opening bracket by one
    |> map2 (\b d -> if b then d - 1 else d) brackets

local let argsort_by_depth [n] (depths: [n]i32): []u32 =
    radix_sort
        u32.num_bits
        (\bit index -> i32.get_bit bit depths[i64.u32 index])
        (iota n |> map u32.i64)

-- Match up pairs of brackets. Opening brackets are represented by `true`, closing
-- brackets by `false`. If the input array size is not divisible by 2, the resulting
-- index array will contain out-of-bounds indices, but otherwise not. If the input
-- array is unbalanced, matches will not pair up.
-- output[i] = j means that the bracket at index i would pair with the bracket at index
-- j if the input is balanced.
-- TODO: Verify what happens if the bracket depth becomes negative. Can an input be
-- constructed which would still return valid even though its depth becomes negative?
-- TODO: This function's memory usage and performance could probably be improved.
let build_bracket_match [n] (brackets: [n]bool): []u32 =
    let depths = compute_depths brackets
    -- At this point, `depths` might contain negative depths if the input is not
    -- balanced.  For now, these are just ignored during the radix sort, but could
    -- be filtered out by simply checking whether the first or last element of the
    -- sorted depths vector is negative.

    -- Build an array of indices sorted by the depth. Because radix sort is stable, this
    -- will return an array of indices where indices of all brackets on the same level
    -- are consequent.
    let sorting = argsort_by_depth depths
    -- In order to build the final array of matches, we're going to scatter the buddy of
    -- each bracket. Since the indices of every bracket and their buddy are now together,
    -- and (if the input is balanced) opening brackets will be on even positions, we can
    -- find the buddy index of each bracket simply by swapping each pair of indices.
    let swapped = map (\i -> sorting[i ^ 1]) (iota n)
    -- Finally, scatter the buddy index to the bracket index to find the final match array
    in scatter
        (replicate n 0u32)
        (map i64.u32 sorting)
        swapped

-- Match up pairs of brackets. Opening brackets are represented by `true`, closing
-- brackets by `false`. If the input array size is not divisible by 2, the resulting
-- index array will contain out-of-bounds indices, but otherwise not. If the input
-- array is unbalanced, matches will not pair up.
-- Returns an array of pairs (i, j) where brackets with indices i and j (with i an opening
-- bracket and j a closing bracket) will pair up if the input is balanced.
-- TODO: Verify what happens if the bracket depth becomes negative. Can an input be
-- constructed which would still return valid even though its depth becomes negative?
-- TODO: This function's memory usage and performance could probably be improved.
let argpair_brackets [n] (brackets: [n]bool): [](u32, u32) =
    let depths = compute_depths brackets
    let sorting = argsort_by_depth depths
    let m = n / 2
    in zip (sorting[0::2] :> [m]u32) (sorting[1::2] :> [m]u32)
