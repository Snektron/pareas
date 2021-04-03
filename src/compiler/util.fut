-- | Utility function to transform an array into an array of subsequent pairs.
-- The dimension of the input array is expected to be even.
let in_pairs [n] 't (xs: [n]t): [](t, t) =
    let m = assert (n % 2 == 0) (n / 2)
    in zip (xs[0::2] :> [m]t) (xs[1::2] :> [m]t)

-- | Utility function to transform an array into an array of windows of each pair
let in_windows_of_pairs [n] 't (xs: [n]t): [](t, t) =
    let m = n - 1
    in zip (xs[:m] :> [m]t) (xs[1:] :> [m]t)

-- | Calculate the number of bits required to store a certain value
-- Returns `i32.num_bits` for negative numbers, and returns 0 for 0.
let bit_width (x: i32): i32 = i32.num_bits - (i32.clz x)

-- | Shift all the elements in xs to the right, and shift x into the
-- left. The right value is discarded.
let shift_right [n] 't (x: t) (xs: [n]t): [n]t =
    xs
    |> rotate (-1)
    |> zip (iota n)
    |> map (\(i, y) -> if i == 0 then x else y)

-- | Shift all the elements in xs to the left, and shift x into
-- the empty spot on the right. The left value is discarded.
let shift_left [n] 't (x: t) (xs: [n]t): [n]t =
    xs
    |> rotate (1)
    |> zip (iota n)
    |> map (\(i, y) -> if i == n - 1 then x else  y)

-- | Perform an exclusive scan. The initial value of the returned array will be ne.
let exclusive_scan [n] 't (op: t -> t -> t) (ne: t) (as: [n]t): [n]t =
    scan op ne as
    |> shift_right ne

-- | Fetch elements from `xs` according to the indices in `is`.
let gather [n] 't (xs: []t) (is: [n]i32): [n]t =
    map (\i -> xs[i]) is
