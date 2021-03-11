module token = u8
let token_plus: token.t = 0
let token_a: token.t = 1
let token_lbracket: token.t = 2
let token_rbracket: token.t = 3
let num_tokens: i64 = 4
let num_table_tokens: i64 = 6
let start_of_input_index: i64 = 4
let end_of_input_index: i64 = 5
module production = u8
let num_productions: i64 = 6
let production_start: production.t = 0
let production_expr: production.t = 1
let production_sum: production.t = 2
let production_sum_end: production.t = 3
let production_literal: production.t = 4
let production_brackets: production.t = 5
let production_arity = [1, 2, 2, 0, 0, 1] :> [num_productions]i32
module bracket = u8
module stack_change_offset = i8
let stack_change_table_size: i64 = 30
let stack_change_table = [2, 0, 2, 3, 5, 6, 3, 1, 7, 9, 3, 9, 3, 1, 7, 2, 0, 6, 3, 2, 3, 5, 4, 1, 7, 4, 2, 8, 2, 8] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (25, 1), (22, 3), (-1, -1), (-1, -1), (-1, -1)],
    [(2, 3), (-1, -1), (-1, -1), (15, 2), (-1, -1), (26, 2)],
    [(-1, -1), (17, 2), (5, 4), (-1, -1), (-1, -1), (-1, -1)],
    [(19, 3), (-1, -1), (-1, -1), (0, 2), (-1, -1), (28, 2)],
    [(-1, -1), (9, 2), (11, 4), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)]
] :> [num_table_tokens][num_table_tokens](i8, i8)
module parse_offset = i8
let parse_table_size: i64 = 18
let parse_table = [production_sum_end, production_sum, production_expr, production_brackets, production_start, production_expr, production_literal, production_start, production_expr, production_brackets, production_sum_end, production_expr, production_literal, production_sum, production_brackets, production_literal, production_sum_end, production_sum_end] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (15, 1), (14, 1), (-1, -1), (-1, -1), (-1, -1)],
    [(1, 1), (-1, -1), (-1, -1), (10, 1), (-1, -1), (16, 1)],
    [(-1, -1), (11, 2), (2, 2), (-1, -1), (-1, -1), (-1, -1)],
    [(13, 1), (-1, -1), (-1, -1), (0, 1), (-1, -1), (17, 1)],
    [(-1, -1), (4, 3), (7, 3), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)]
] :> [num_table_tokens][num_table_tokens](i8, i8)
