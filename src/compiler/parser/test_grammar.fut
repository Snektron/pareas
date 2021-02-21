type production = #start | #expr | #sum | #sum_end | #literal | #brackets
type token = #rbracket | #lbracket | #a | #plus | #eoi | #soi
module bracket = u8
module stack_change_offset = i8
let stack_change_table_size: i64 = 30
let stack_change_table = [2, 0, 5, 3, 2, 0, 2, 3, 7, 2, 3, 7, 8, 3, 1, 9, 6, 8, 3, 2, 4, 6, 1, 9, 2, 4, 5, 3, 1, 9] :> [stack_change_table_size]u8
let get_stack_change (a: token) (b: token): (i8, i8) =
    match (a, b)
    case (#soi, #lbracket) -> (26, 4)
    case (#rbracket, #eoi) -> (24, 2)
    case (#plus, #lbracket) -> (21, 3)
    case (#a, #eoi) -> (19, 2)
    case (#lbracket, #a) -> (17, 2)
    case (#plus, #a) -> (16, 1)
    case (#lbracket, #lbracket) -> (12, 4)
    case (#rbracket, #plus) -> (9, 3)
    case (#a, #plus) -> (6, 3)
    case (#a, #rbracket) -> (4, 2)
    case (#soi, #a) -> (2, 2)
    case (#rbracket, #rbracket) -> (0, 2)
    case _ -> (-1, -1)
module parse_offset = i8
let parse_table_size: i64 = 18
let parse_table = [#sum_end, #start, #expr, #literal, #sum_end, #sum, #sum, #expr, #brackets, #literal, #expr, #literal, #sum_end, #brackets, #sum_end, #start, #expr, #brackets] :> [parse_table_size]production
let get_parse (a: token) (b: token): (i8, i8) =
    match (a, b)
    case (#soi, #lbracket) -> (15, 3)
    case (#rbracket, #eoi) -> (14, 1)
    case (#plus, #lbracket) -> (13, 1)
    case (#a, #eoi) -> (12, 1)
    case (#lbracket, #a) -> (10, 2)
    case (#plus, #a) -> (9, 1)
    case (#lbracket, #lbracket) -> (7, 2)
    case (#rbracket, #plus) -> (6, 1)
    case (#a, #plus) -> (5, 1)
    case (#a, #rbracket) -> (4, 1)
    case (#soi, #a) -> (1, 3)
    case (#rbracket, #rbracket) -> (0, 1)
    case _ -> (-1, -1)
