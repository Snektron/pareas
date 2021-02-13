-- | ignore
import "bracket_matching"

-- [ ( ) { ( ) } ]
-- 0 1 2 3 4 5 6 7 -- index
-- 1 2 1 2 3 2 1 0 -- prefix
-- 0 1 1 1 2 2 1 0 -- depth
-- 0 7 1 2 3 6 4 5 -- sort over indices
-- 7 0 2 1 6 3 5 4 -- swap pairs
-- 7 2 1 6 5 4 3 0 -- scatter
-- [ ] ( ) { } ( )
-- ==
-- entry: test_build_bracket_match
-- input { [true, true, false, true, true, false, false, false] }
-- output { [7u32, 2u32, 1u32, 6u32, 5u32, 4u32, 3u32, 0u32] }
let test_build_bracket_match = build_bracket_match

