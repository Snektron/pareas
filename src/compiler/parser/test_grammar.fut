module production = u8
let num_productions: i64 = 6
let production_brackets: production.t = 5
let production_literal: production.t = 4
let production_sum_end: production.t = 3
let production_sum: production.t = 2
let production_expr: production.t = 1
let production_start: production.t = 0
let production_arity = [1, 2, 2, 0, 0, 1] :> [num_productions]i32
module token = u8
let num_tokens: i64 = 6
let token_rbracket: token.t = 5
let token_lbracket: token.t = 4
let token_a: token.t = 3
let token_plus: token.t = 2
let special_token_eoi: token.t = 1
let special_token_soi: token.t = 0
module bracket = u8
module stack_change_offset = i8
let stack_change_table_size: i64 = 30
let stack_change_table = [0, 3, 2, 4, 0, 3, 5, 1, 2, 3, 7, 9, 3, 9, 3, 5, 1, 6, 5, 1, 6, 2, 8, 2, 8, 2, 3, 7, 2, 4] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (-1, -1), (-1, -1), (11, 2), (13, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (20, 1), (17, 3), (-1, -1)],
    [(-1, -1), (21, 2), (8, 3), (-1, -1), (-1, -1), (2, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (0, 2), (4, 4), (-1, -1)],
    [(-1, -1), (23, 2), (25, 3), (-1, -1), (-1, -1), (28, 2)]
] :> [num_tokens][num_tokens](i8, i8)
module parse_offset = i8
let parse_table_size: i64 = 18
let parse_table = [production_expr, production_literal, production_sum_end, production_expr, production_brackets, production_sum, production_start, production_expr, production_literal, production_start, production_expr, production_brackets, production_brackets, production_literal, production_sum_end, production_sum_end, production_sum, production_sum_end] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (-1, -1), (-1, -1), (6, 3), (9, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (13, 1), (12, 1), (-1, -1)],
    [(-1, -1), (14, 1), (5, 1), (-1, -1), (-1, -1), (2, 1)],
    [(-1, -1), (-1, -1), (-1, -1), (0, 2), (3, 2), (-1, -1)],
    [(-1, -1), (15, 1), (16, 1), (-1, -1), (-1, -1), (17, 1)]
] :> [num_tokens][num_tokens](i8, i8)
