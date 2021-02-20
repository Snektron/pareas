-- Utility function to transform an array into an array of subsequent pairs.
-- The dimension of the input array is expected to be even.
let in_pairs [n] 't (xs: [n]t): [](t, t) =
    let m = assert (n % 2 == 0) (n / 2)
    in zip (xs[0::2] :> [m]t) (xs[1::2] :> [m]t)

-- Calculate the number of bits required to store a certain value
-- Returns `i32.num_bits` for negative numbers, and returns 0 for 0.
let bit_width (x: i32): i32 = i32.num_bits - (i32.clz x)
