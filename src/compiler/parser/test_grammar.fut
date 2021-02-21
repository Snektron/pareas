module production = u8
let production_brackets: production.t = 5
let production_literal: production.t = 4
let production_sum_end: production.t = 3
let production_sum: production.t = 2
let production_expr: production.t = 1
let production_start: production.t = 0
module token = u8
let num_tokens: i64 = 6
let token_rbracket: token.t = 5
let token_lbracket: token.t = 4
let token_a: token.t = 3
let token_plus: token.t = 2
let token_eoi: token.t = 1
let token_soi: token.t = 0
module bracket = u8
module stack_change_offset = i8
let stack_change_table_size: i64 = 30
let stack_change_table = [0, 3, 2, 4, 0, 3, 7, 1, 8, 2, 3, 9, 8, 7, 1, 2, 3, 9, 2, 6, 2, 4, 5, 3, 2, 6, 5, 3, 7, 1] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (-1, -1), (-1, -1), (22, 2), (26, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (8, 1), (12, 3), (-1, -1)],
    [(-1, -1), (2, 2), (15, 3), (-1, -1), (-1, -1), (18, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (0, 2), (4, 4), (-1, -1)],
    [(-1, -1), (20, 2), (9, 3), (-1, -1), (-1, -1), (24, 2)]
] :> [num_tokens][num_tokens](i8, i8)
module parse_offset = i8
let parse_table_size: i64 = 18
let parse_table = [production_expr, production_literal, production_sum_end, production_expr, production_brackets, production_literal, production_sum, production_brackets, production_sum, production_sum_end, production_sum_end, production_start, production_expr, production_literal, production_sum_end, production_start, production_expr, production_brackets] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (-1, -1), (-1, -1), (11, 3), (15, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (5, 1), (7, 1), (-1, -1)],
    [(-1, -1), (2, 1), (8, 1), (-1, -1), (-1, -1), (9, 1)],
    [(-1, -1), (-1, -1), (-1, -1), (0, 2), (3, 2), (-1, -1)],
    [(-1, -1), (10, 1), (6, 1), (-1, -1), (-1, -1), (14, 1)]
] :> [num_tokens][num_tokens](i8, i8)
