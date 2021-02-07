-- | ignore
import "llpgen_rt"

-- ==
-- entry: test_pack_nonempty_strings
-- input { [0u8, 1u8, 2u8, 3u8, 4u8, 5u8, 6u8, 7u8, 8u8, 9u8, 10u8]
--         [1i64, 8i64, 5i64, 2i64, 4i64]
--         [3i64, 2i64, 4i64, 1i64, 2i64] }
-- output { [1u8, 2u8, 3u8, 8u8, 9u8, 5u8, 6u8, 7u8, 8u8, 2u8, 4u8, 5u8] }
entry test_pack_nonempty_strings [n] (text: []u8) (is: [n]i64) (lens: [n]i64) =
    pack_nonempty_strings text is lens

-- ==
-- entry: test_pack_strings
-- input { [0u8, 1u8, 2u8, 3u8, 4u8, 5u8, 6u8, 7u8, 8u8, 9u8, 10u8]
--         [1i64, 3i64, 8i64, 5i64, 1i64, 2i64, 0i64, 4i64]
--         [3i64, 0i64, 2i64, 4i64, 0i64, 1i64, 0i64, 2i64] }
-- output { [1u8, 2u8, 3u8, 8u8, 9u8, 5u8, 6u8, 7u8, 8u8, 2u8, 4u8, 5u8] }
entry test_pack_strings [n] (text: []u8) (is: [n]i64) (lens: [n]i64) =
    pack_strings text is lens
