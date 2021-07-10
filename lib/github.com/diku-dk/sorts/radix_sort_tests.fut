-- | ignore

import "radix_sort"

-- ==
-- entry: sort_i32
-- input { [5,4,3,2,1,0,-1,-2] }
-- output { [-2,-1,0,1,2,3,4,5] }
-- input { [5,4,3,3,2,1,0,-1,-2,-1] }
-- output { [-2,-1,-1,0,1,2,3,3,4,5] }

entry sort_i32 = radix_sort_int i32.num_bits i32.get_bit

-- ==
-- entry: sort_u16
-- input { [5u16,4u16,3u16,2u16,1u16,-1u16] }
-- output { [-1u16,1u16,2u16,3u16,4u16,5u16] }

entry sort_u16 = radix_sort_int u16.num_bits u16.get_bit

-- ==
-- entry: sort_f32
-- input { [5f32,4f32,3f32,2f32,-1f32,-2f32] }
-- output { [-2f32,-1f32,2f32,3f32,4f32,5f32] }

entry sort_f32 = radix_sort_float f32.num_bits f32.get_bit

-- ==
-- entry: sort_perm_i32
-- input { [5,4,3,2,1,0,-1,-2] }
-- output { [7, 6, 5, 4, 3, 2, 1, 0] }

entry sort_perm_i32 [n] (xs: [n]i32) =
  zip xs (iota n)
  |> radix_sort_int_by_key (.0) i32.num_bits i32.get_bit
  |> map ((.1) >-> i32.i64)

-- ==
-- entry: sort_perm_f32
-- input { [5f32,4f32,3f32,2f32,1f32,0f32,-1f32,-2f32] }
-- output { [7, 6, 5, 4, 3, 2, 1, 0] }

entry sort_perm_f32 [n] (xs: [n]f32) =
  zip xs (iota n)
  |> radix_sort_float_by_key (.0) f32.num_bits f32.get_bit
  |> map ((.1) >-> i32.i64)
