import "util"

-- | Extract strings defined by an array of offsets and an array of lengths from text,
-- and pack the results into a new array.
let extract 't [n] (text: []t) (offsets: [n]i32) (lens: [n]i32): *[]t =
    -- Create an array of indices which will be used to index text
    -- Each string consisting of (offset, len) will be gathered by constructing
    -- runs of indices.
    -- Starts of strings will be replaced with the difference between the end of the
    -- previous run of indices and the start of the next. The final gather indices will
    -- then be obtained by scattering these differences in an array of ones and computing
    -- a prefix sum over the result.
    let m = reduce (+) 0 lens
    let dest = replicate (i64.i32 m) 1
    -- Compute the first indices of each string in the final array
    let scatter_indices = exclusive_scan (+) 0 lens
    let scatter_diffs =    -- Compute an array of differences between the end of the previous run and
    -- the start of the next run.
        map2 (+) offsets lens
        |> map (+ -1)
        |> shift_right 0
        |> map2 (-) offsets
        -- Subtract one to account for the ones in the existing array
        |> map (+ -1)
    -- Compute the final array of indices
    let gather_indices =
        reduce_by_index dest (+) 0 (map i64.i32 scatter_indices) scatter_diffs
        |> scan (+) 0
    -- Finally, perform the gather
    in map (\i -> text[i]) gather_indices
