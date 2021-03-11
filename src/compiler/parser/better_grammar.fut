module token = u8
let token_fn: token.t = 0
let token_while: token.t = 1
let token_if: token.t = 2
let token_semi: token.t = 3
let token_lbrace: token.t = 4
let token_rbrace: token.t = 5
let token_plus: token.t = 6
let token_minus: token.t = 7
let token_star: token.t = 8
let token_slash: token.t = 9
let token_id: token.t = 10
let token_lparen: token.t = 11
let token_rparen: token.t = 12
let num_tokens: i64 = 13
let num_table_tokens: i64 = 15
let start_of_input_index: i64 = 13
let end_of_input_index: i64 = 14
module production = u8
let num_productions: i64 = 22
let production_start: production.t = 0
let production_program: production.t = 1
let production_program_end: production.t = 2
let production_fn_decl: production.t = 3
let production_stat_while: production.t = 4
let production_stat_if: production.t = 5
let production_stat_expr: production.t = 6
let production_stat_compound: production.t = 7
let production_compound_stat: production.t = 8
let production_stat_list: production.t = 9
let production_stat_list_end: production.t = 10
let production_expr: production.t = 11
let production_sum: production.t = 12
let production_sum_add: production.t = 13
let production_sum_sub: production.t = 14
let production_sum_end: production.t = 15
let production_prod: production.t = 16
let production_prod_mul: production.t = 17
let production_prod_div: production.t = 18
let production_prod_end: production.t = 19
let production_atom_variable: production.t = 20
let production_atom_paren: production.t = 21
let production_arity = [1, 2, 0, 1, 2, 2, 1, 1, 1, 2, 0, 1, 2, 2, 2, 0, 2, 2, 2, 0, 0, 1] :> [num_productions]i32
module bracket = u8
module stack_change_offset = i16
let stack_change_table_size: i64 = 183
let stack_change_table = [0, 1, 3, 5, 7, 0, 1, 9, 1, 6, 4, 2, 10, 9, 1, 0, 1, 3, 5, 7, 13, 15, 6, 4, 2, 16, 13, 15, 6, 4, 5, 19, 6, 4, 10, 9, 1, 14, 5, 7, 0, 1, 9, 1, 6, 4, 5, 19, 16, 16, 13, 15, 16, 0, 1, 11, 15, 6, 7, 17, 14, 5, 7, 21, 23, 11, 6, 4, 12, 0, 1, 9, 1, 0, 1, 11, 15, 18, 7, 13, 15, 14, 5, 7, 13, 15, 0, 1, 3, 5, 7, 0, 1, 11, 15, 18, 7, 0, 8, 0, 1, 3, 5, 7, 13, 15, 18, 7, 13, 15, 18, 7, 0, 1, 3, 5, 7, 13, 15, 0, 1, 11, 15, 0, 1, 11, 15, 6, 4, 10, 9, 1, 0, 1, 3, 5, 7, 22, 23, 11, 14, 5, 7, 0, 8, 6, 4, 5, 19, 6, 7, 17, 0, 8, 14, 5, 7, 13, 15, 6, 4, 5, 19, 14, 5, 7, 13, 15, 22, 20, 0, 1, 11, 15, 6, 7, 17, 6, 7, 17, 6, 4, 12] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (12, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (140, 3), (81, 5), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (37, 3), (163, 5), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (119, 4), (91, 4), (-1, -1), (69, 4), (143, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (132, 5), (112, 7), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (123, 4), (73, 4), (-1, -1), (40, 4), (97, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (86, 5), (15, 7), (-1, -1), (-1, -1), (-1, -1)],
    [(137, 3), (53, 4), (170, 4), (-1, -1), (5, 4), (152, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 5), (99, 7), (-1, -1), (-1, -1), (168, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (95, 2), (77, 4), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (110, 2), (106, 4), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (52, 1), (49, 3), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (48, 1), (25, 3), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (9, 3), (32, 5), (-1, -1), (145, 4), (159, 4), (57, 3), (174, 3), (-1, -1), (-1, -1), (66, 3), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (60, 3), (154, 5), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (22, 3), (127, 5), (-1, -1), (44, 4), (28, 4), (149, 3), (177, 3), (-1, -1), (-1, -1), (180, 3), (-1, -1), (-1, -1)],
    [(63, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (25, 0)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)]
] :> [num_table_tokens][num_table_tokens](i16, i16)
module parse_offset = i16
let parse_table_size: i64 = 131
let parse_table = [production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_prod_end, production_sum_end, production_compound_stat, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod_end, production_sum_end, production_start, production_program_end, production_atom_paren, production_prod_end, production_sum_sub, production_prod_end, production_sum_end, production_compound_stat, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_prod_end, production_sum_add, production_atom_variable, production_atom_paren, production_atom_variable, production_stat_list, production_stat_while, production_prod_mul, production_expr, production_sum, production_prod, production_atom_variable, production_start, production_program, production_fn_decl, production_prod_end, production_sum_end, production_stat_list, production_stat_compound, production_compound_stat, production_stat_list, production_stat_if, production_prod, production_atom_paren, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_if, production_prod, production_atom_variable, production_stat_list_end, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod, production_atom_paren, production_prod, production_atom_variable, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list, production_stat_while, production_stat_list, production_stat_while, production_prod_end, production_sum_end, production_compound_stat, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_program, production_fn_decl, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list_end, production_prod_end, production_sum_add, production_prod_mul, production_stat_list_end, production_expr, production_sum, production_prod, production_atom_paren, production_prod_end, production_sum_sub, production_expr, production_sum, production_prod, production_atom_paren, production_program_end, production_stat_list, production_stat_if, production_prod_div, production_prod_div, production_prod_end, production_sum_end] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (11, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (105, 4), (59, 4), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (28, 4), (120, 4), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (90, 2), (69, 2), (-1, -1), (52, 3), (109, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (97, 6), (84, 6), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (92, 2), (55, 2), (-1, -1), (32, 3), (73, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (63, 6), (12, 6), (-1, -1), (-1, -1), (-1, -1)],
    [(103, 2), (40, 2), (125, 2), (-1, -1), (6, 3), (113, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 6), (74, 6), (-1, -1), (-1, -1), (124, 1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (71, 2), (57, 2), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (82, 2), (80, 2), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (39, 1), (38, 1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (37, 1), (22, 1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (9, 2), (25, 3), (-1, -1), (110, 2), (118, 2), (42, 1), (127, 1), (-1, -1), (-1, -1), (50, 2), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (43, 4), (114, 4), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (18, 2), (94, 3), (-1, -1), (35, 2), (23, 2), (112, 1), (128, 1), (-1, -1), (-1, -1), (129, 2), (-1, -1), (-1, -1)],
    [(47, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (20, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)]
] :> [num_table_tokens][num_table_tokens](i16, i16)
