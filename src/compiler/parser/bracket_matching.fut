import "../../../lib/github.com/diku-dk/sorts/radix_sort"
import "../util"
module bt = import "binary_tree"

-- Build an array that for every bracket gives its nesting depth. Opening brackets are
-- represented by `true` and closing brackets by `false`.
local let compute_depths [n] (brackets: [n]bool): []i32 =
    brackets
    -- Map each bracket to a stack depth change
    |> map (\b -> if b then 1i32 else -1i32)
    -- Perform prefix sum to build the initial depth vector
    |> scan (+) 0
    -- This will result in the right depth for closing parenthesis,
    -- but not for the opening parenthesis:
    -- [ ( ) { ( ) } ]
    -- 1 2 1 2 3 2 1 0 - we have this after the scan
    -- 0 1 1 1 2 2 1 0 - we want to get this
    -- We fix this by simply decreasing the depth of each opening bracket by one
    |> map2 (\b d -> d - i32.bool b) brackets

-- Given a function determining whether a bracket is open or closing, a function to check if two
-- brackets form a matching pair, and an array of brackets, this function returns whether
-- the array of brackets is balanced. This function also accounts for negative depths.
-- This version uses a binary tree to check brackets. This seems to be faster than the radix sort.
let check_brackets_bt [n] 'b (is_open: b -> bool) (is_pair: b -> b -> bool) (brackets: [n]b) =
    -- Early return if the size is uneven: these would never be able to pair up
    if n % 2 != 0 then false else
    -- Compute a bit mask of which brackets are open brackets
    let opens = map is_open brackets
    -- Compute depths
    -- In order to transform the bracket matching problem to a previous-smaller-value problem,
    -- each left parenthesis is mapped to an even value one less than the right parenthesis,
    -- which are all mapped to odd values. This way, the previous value smaller than right brackets
    -- will always be the left bracket, _iff_ the brackets are balanced.
    let depths =
        compute_depths opens
        -- Perform the mapping
        |> map2 (\b d -> 2 * d + if b then 0 else 1) opens
    -- Calculate the minimum depth to verify that the depth doesn't go negative.
    let min_depth = reduce (i32.min) 0 depths
    -- Early return if the depth does negative or the brackets aren't balanced regarding only
    -- opens and closes.
    in if min_depth < 0 || last depths != 1 then false else
    -- Construct the binary tree
    -- TODO: This constructs a full binary tree, and copies the leaves (the depths) also in it.
    -- That could probably be improved memory-wise.
    let tree = bt.construct i32.min i32.highest depths
    in map3
        -- For each right bracket, find the left bracket and check whether they form a pair
        -- Skip looking up the mate for left brackets as well
        (\i o b -> o || let m = bt.find_psv tree i in m >= 0 && is_pair brackets[m] b)
        (iota n |> map i32.i64)
        opens
        brackets
    -- Finally, check whether they all match up
    |> all id

-- Given a function determining whether a bracket is open or closing, a function to check if two
-- brackets form a matching pair, and an array of brackets, this function returns whether
-- the array of brackets is balanced. This function also accounts for negative depths.
-- This function uses a radix sort to check for brackets.
let check_brackets_radix [n] 'b (is_open: b -> bool) (is_pair: b -> b -> bool) (brackets: [n]b): bool =
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
    let bits = bit_width max_depth
    in zip depths brackets
        -- Sort the combined depth/brackets array by depth
        |> radix_sort bits (\bit (depth, _) -> i32.get_bit bit depth)
        -- Discard depths
        |> map (\(_, bracket) -> bracket)
        |> in_pairs
        -- Check if each two brackets form a pair
        |> all (\(a, b) -> is_pair a b)
