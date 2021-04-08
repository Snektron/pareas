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

-- | Quick implementation of a 4-way partition (keeping in style with partition2 which splits into
-- 3 groups). `p` should yield 0-3, the partition which this element should be put in.
-- This function preserves relative ordering between elements within partitions.
let partition3 [n] 't (p: t -> i32) (ts: [n]t): ([]t, []t, []t, []t) =
    -- This is mostly taken from `radix_sort_step`. It could be implemented with a radix sort, however,
    -- for the final split into different arrays we need the counts.
    let pairwise op (a1, b1, c1, d1) (a2, b2, c2, d2) = (op a1 a2, op b1 b2, op c1 c2, op d1 d2)
    let bins = map p ts
    let offsets =
        bins
        |> map (\bin ->
            if bin == 0 then (1, 0, 0, 0)
            else if bin == 1 then (0, 1, 0, 0)
            else if bin == 2 then (0, 0, 1, 0)
            else (0, 0, 0, 1))
        |> scan (pairwise (+)) (0, 0, 0, 0)
    let (na, nb, nc, _) = last offsets
    let f bin (a, b, c, d) =
        match bin
        case 0 -> a - 1
        case 1 -> na + b - 1
        case 2 -> na + nb + c - 1
        case _ -> na + nb + nc + d - 1
    let is = map2 f bins offsets
    let ts' = scatter (copy ts) is ts
    in (ts'[0 : na], ts'[na : na + nb], ts'[na + nb : na + nb + nc], ts'[na + nb + nc : n])
