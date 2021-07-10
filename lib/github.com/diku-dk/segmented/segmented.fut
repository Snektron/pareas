-- | Irregular segmented operations, like scans and reductions.

-- | Segmented scan. Given a binary associative operator ``op`` with
-- neutral element ``ne``, computes the inclusive prefix scan of the
-- segments of ``as`` specified by the ``flags`` array, where `true`
-- starts a segment and `false` continues a segment.
let segmented_scan [n] 't (op: t -> t -> t) (ne: t)
                          (flags: [n]bool) (as: [n]t): [n]t =
  (unzip (scan (\(x_flag,x) (y_flag,y) ->
                (x_flag || y_flag,
                 if y_flag then y else x `op` y))
          (false, ne)
          (zip flags as))).1

-- | Segmented reduction. Given a binary associative operator ``op``
-- with neutral element ``ne``, computes the reduction of the segments
-- of ``as`` specified by the ``flags`` array, where `true` starts a
-- segment and `false` continues a segment.  One value is returned per
-- segment.
let segmented_reduce [n] 't (op: t -> t -> t) (ne: t)
                            (flags: [n]bool) (as: [n]t) =
  -- Compute segmented scan.  Then we just have to fish out the end of
  -- each segment.
  let as' = segmented_scan op ne flags as
  -- Find the segment ends.
  let segment_ends = rotate 1 flags
  -- Find the offset for each segment end.
  let segment_end_offsets = segment_ends |> map i64.bool |> scan (+) 0
  let num_segments = if n > 0 then last segment_end_offsets else 0
  -- Make room for the final result.  The specific value we write here
  -- does not matter; they will all be overwritten by the segment
  -- ends.
  let scratch = replicate num_segments ne
  -- Compute where to write each element of as'.  Only segment ends
  -- are written.
  let index i f = if f then i-1 else -1
  in scatter scratch (map2 index segment_end_offsets segment_ends) as'

-- | Replicated iota. Given a repetition array, the function returns
-- an array with each index (starting from 0) repeated according to
-- the repetition array. As an example, replicated_iota [2,3,1]
-- returns the array [0,0,1,1,1,2].

let replicated_iota [n] (reps:[n]i64) : []i64 =
  let s1 = scan (+) 0 reps
  let s2 = map2 (\i x -> if i==0 then 0 else x)
                (iota n) (rotate (-1) s1)
  let tmp = reduce_by_index (replicate (reduce (+) 0 reps) 0) i64.max 0 s2 (iota n)
  let flags = map (>0) tmp
  in segmented_scan (+) 0 flags tmp

-- | Segmented iota. Given a flags array, the function returns an
-- array of index sequences, each of which is reset according to the
-- flags array. As an examples, segmented_iota
-- [false,false,false,true,false,false,false] returns the array
-- [0,1,2,0,1,2,3].

let segmented_iota [n] (flags:[n]bool) : [n]i64 =
  let iotas = segmented_scan (+) 0 flags (replicate n 1)
  in map (\x -> x-1) iotas

-- | Generic expansion function. The function expands a source array
-- into a target array given (1) a function that determines, for each
-- source element, how many target elements it expands to and (2) a
-- function that computes a particular target element based on a
-- source element and the target element number associated with the
-- source. As an example, the expression expand (\x->x) (*) [2,3,1]
-- returns the array [0,2,0,3,6,0].

let expand 'a 'b (sz: a -> i64) (get: a -> i64 -> b) (arr:[]a) : []b =
  let szs = map sz arr
  let idxs = replicated_iota szs
  let iotas = segmented_iota (map2 (!=) idxs (rotate (-1) idxs))
  in map2 (\i j -> get arr[i] j) idxs iotas

-- | Expansion function equivalent to performing a segmented reduction
-- to the result of a general expansion with a flags vector expressing
-- the beginning of the expanded segments. The function makes use of
-- the intermediate flags vector generated as part of the expansion
-- and the `expand_reduce` function is therefore more efficient than
-- if a segmented reduction (with an appropriate flags vector) is
-- explicitly followed by a call to expand.

let expand_reduce 'a 'b (sz: a -> i64) (get: a -> i64 -> b)
                        (op: b -> b -> b) (ne:b) (arr:[]a) : []b =
  let szs = map sz arr
  let idxs = replicated_iota szs
  let flags = map2 (!=) idxs (rotate (-1) idxs)
  let iotas = segmented_iota flags
  let vs = map2 (\i j -> get arr[i] j) idxs iotas
  in segmented_reduce op ne flags vs

-- | Expansion followed by an ''outer segmented reduce'' that ensures
-- that each element in the result array corresponds to expanding and
-- reducing the corresponding element in the source array.

let expand_outer_reduce 'a 'b [n] (sz: a -> i64) (get: a -> i64 -> b)
                                  (op: b -> b -> b) (ne: b)
                                  (arr: [n]a) : [n]b =
  let sz' x = let s = sz x
              in if s == 0 then 1 else s
  let get' x i = if sz x == 0 then ne else get x i
  in expand_reduce sz' get' op ne arr :> [n]b
