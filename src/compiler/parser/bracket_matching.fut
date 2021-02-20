import "../../../lib/github.com/diku-dk/sorts/radix_sort"
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

-- Utility function to transform an array into an array of subsequent pairs.
-- The dimension of the input array is expected to be even.
local let in_pairs [n] 't (xs: [n]t): [](t, t) =
    let m = assert (n % 2 == 0) (n / 2)
    in zip (xs[0::2] :> [m]t) (xs[1::2] :> [m]t)

-- Given a function determining whether a bracket is open or closing, a function to check if two
-- brackets form a matching pair, and an array of brackets, this function returns whether
-- the array of brackets is balanced. This function also accounts for negative depths.
let check_brackets [n] 'b (is_open: b -> bool) (is_pair: b -> b -> bool) (brackets: [n]b) =
    if n % 2 != 0 then false else
    let opens = map is_open brackets
    let depths =
        compute_depths opens
        |> map2 (\b d -> 2 * d + if b then 1 else 2) opens
    in if last depths != 2 then false else
    let tree = bt.construct i32.min i32.highest depths
    let psv = map (bt.find_psv tree) (iota n |> map i32.i64)
    in map3
        (\i left b ->
            if left
            then true
            else i >= 0 && is_pair brackets[i] b)
        psv
        opens
        brackets
    |> all id
