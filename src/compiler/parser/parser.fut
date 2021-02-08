import "string_packing"

module type Grammar = {
    type production

    val stack_change_table_size: i64
    val stack_change_table: [stack_change_table_size]u8
    val initial_stack: (i64, i64)
    val get_stack_change 'terminal: terminal -> terminal -> (i64, i64)

    val parse_table_size: i64
    val parse_table: [parse_table_size]production
    val initial_parse: (i64, i64)
    val get_partial_parse 'terminal: terminal -> terminal -> (i64, i64)
}

-- type production = #start | #expr | #sum | #sum_end | #literal | #brackets
-- let stack_change_table: []u8 = [3, 5, 6, 2, 6, 2, 4, 7, 6, 7, 9, 4, 7, 11, 5, 8, 6, 7, 9, 6, 10, 4, 7, 6, 10, 4, 7, 11, 5, 8, 11, 5]
-- let initial_stack: (i64, i64) = (0, 2)
-- let get_stack_change 'terminal (a: terminal) (b: terminal): (i64, i64) =
--     match (a, b)
--     case (#rbracket, #eoi) -> (2, 2)
--     case (#a, #eoi) -> (4, 2)
--     case (#lbracket, #a) -> (6, 2)
--     case (#rbracket, #plus) -> (8, 3)
--     case (#lbracket, #lbracket) -> (11, 4)
--     case (#plus, #a) -> (15, 1)
--     case (#a, #plus) -> (16, 3)
--     case (#a, #rbracket) -> (19, 2)
--     case (#soi, #a) -> (21, 2)
--     case (#rbracket, #rbracket) -> (23, 2)
--     case (#soi, #lbracket) -> (25, 4)
--     case (#plus, #lbracket) -> (29, 3)
--     case _ -> (-1, -1)
-- let parse_table: []production = [#start, #sum_end, #sum_end, #expr, #literal, #sum, #expr, #brackets, #literal, #sum, #sum_end, #expr, #literal, #sum_end, #expr, #brackets, #brackets]
-- let initial_parse: (i64, i64) = (0, 1)
-- let get_partial_parse 'terminal (a: terminal) (b: terminal): (i64, i64) =
--     match (a, b)
--     case (#rbracket, #eoi) -> (1, 1)
--     case (#a, #eoi) -> (2, 1)
--     case (#lbracket, #a) -> (3, 2)
--     case (#rbracket, #plus) -> (5, 1)
--     case (#lbracket, #lbracket) -> (6, 2)
--     case (#plus, #a) -> (8, 1)
--     case (#a, #plus) -> (9, 1)
--     case (#a, #rbracket) -> (10, 1)
--     case (#soi, #a) -> (11, 2)
--     case (#rbracket, #rbracket) -> (13, 1)
--     case (#soi, #lbracket) -> (14, 2)
--     case (#plus, #lbracket) -> (16, 1)
--     case _ -> (-1, -1)