let pack_nonempty_strings [n] (text: []u8) (offsets: [n]i64) (lens: [n]i64): *[]u8 =
    let ends = map2 (+) offsets lens
        |> map (+ -1)
    let diffs = map
        (\i ->
            if i == 0
            then offsets[0]
            else offsets[i] - ends[i - 1])
        (iota n)
    let scatter_indices = scan (+) 0 lens
        |> rotate (-1)
        |> map2 (\i x -> if i == 0 then 0 else x) (iota n)
    let dest = replicate (reduce (+) 0 lens) 1
    let gather_indices = scatter dest scatter_indices diffs
        |> scan (+) 0
    in map (\i -> text[i]) gather_indices
