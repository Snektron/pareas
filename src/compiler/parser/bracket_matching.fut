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

-- Utility function to transform an array into an array of subsequent pairs.
-- The dimension of the input array is expected to be even.
local let in_pairs [n] 't (xs: [n]t): [](t, t) =
    let m = assert (n % 2 == 0) (n / 2)
    in zip (xs[0::2] :> [m]t) (xs[1::2] :> [m]t)

-- Given a function determining whether a bracket is open or closing, a function to check if two
-- brackets form a matching pair, and an array of brackets, this function returns whether
-- the array of brackets is balanced. This function also accounts for negative depths.
let check_brackets [n] 'b (is_open: b -> bool) (is_pair: b -> b -> bool) (brackets: [n]b): bool =
    -- Early return if the size is uneven: these would never be able to pair up
    if n % 2 != 0 then false else
    -- Compute nesting depth array of the brackets.
    let depths = compute_depths (map is_open brackets)
    -- Compute depth bounds. The min depth is simply used as a check, and the max depth
    -- will be used to bound the radix sort.
    let min_depth = reduce (i32.min) 0 depths
    let max_depth = reduce (i32.max) 0 depths
    -- Early return if the stack size reaches a negative size.
    in if min_depth < 0 then false else
    -- Calculate the amount of bits required to store the depth
    let bits = i32.num_bits - (i32.clz max_depth)
    in
        zip depths brackets
        -- Sort the combined depth/brackets array by depth
        |> radix_sort bits (\bit (depth, _) -> i32.get_bit bit depth)
        -- Discard depths
        |> map (\(_, bracket) -> bracket)
        |> in_pairs
        -- Check if each two brackets form a pair
        |> all (\(a, b) -> is_pair a b)
