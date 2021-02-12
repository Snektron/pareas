type production = #start | #expr | #sum | #sum_end | #literal | #brackets
type token = #rbracket | #lbracket | #a | #plus | #eoi | #soi
module bracket = u8
module stack_change_offset = i8
let stack_change_table_size: i64 = 32
let stack_change_table = [3, 5, 8, 6, 4, 9, 8, 6, 8, 9, 11, 8, 9, 11, 10, 4, 9, 7, 5, 10, 7, 5, 8, 2, 4, 9, 8, 2, 4, 9, 7, 5] :> [stack_change_table_size]u8
let initial_stack_change: (i8, i8) = (0, 2)
let get_stack_change (a: token) (b: token): (i8, i8) =
    match (a, b)
    case (#soi, #lbracket) -> (28, 4)
    case (#rbracket, #eoi) -> (26, 2)
    case (#lbracket, #a) -> (24, 2)
    case (#a, #eoi) -> (22, 2)
    case (#plus, #lbracket) -> (19, 3)
    case (#lbracket, #lbracket) -> (15, 4)
    case (#plus, #a) -> (14, 1)
    case (#rbracket, #plus) -> (11, 3)
    case (#a, #plus) -> (8, 3)
    case (#a, #rbracket) -> (6, 2)
    case (#soi, #a) -> (4, 2)
    case (#rbracket, #rbracket) -> (2, 2)
    case _ -> (-1, -1)
module parse_offset = i8
let parse_table_size: i64 = 17
let parse_table = [#start, #sum_end, #expr, #literal, #sum_end, #sum, #sum, #literal, #expr, #brackets, #brackets, #sum_end, #expr, #literal, #sum_end, #expr, #brackets] :> [parse_table_size]production
let initial_parse: (i8, i8) = (0, 1)
let get_parse (a: token) (b: token): (i8, i8) =
    match (a, b)
    case (#soi, #lbracket) -> (15, 2)
    case (#rbracket, #eoi) -> (14, 1)
    case (#lbracket, #a) -> (12, 2)
    case (#a, #eoi) -> (11, 1)
    case (#plus, #lbracket) -> (10, 1)
    case (#lbracket, #lbracket) -> (8, 2)
    case (#plus, #a) -> (7, 1)
    case (#rbracket, #plus) -> (6, 1)
    case (#a, #plus) -> (5, 1)
    case (#a, #rbracket) -> (4, 1)
    case (#soi, #a) -> (2, 2)
    case (#rbracket, #rbracket) -> (1, 1)
    case _ -> (-1, -1)
