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
module token = u8
let num_tokens: i64 = 15
let token_rparen: token.t = 14
let special_token_soi: token.t = 0
let special_token_eoi: token.t = 1
let token_lparen: token.t = 13
let token_if: token.t = 4
let token_slash: token.t = 11
let token_fn: token.t = 2
let token_plus: token.t = 8
let token_lbrace: token.t = 6
let token_rbrace: token.t = 7
let token_while: token.t = 3
let token_minus: token.t = 9
let token_star: token.t = 10
let token_semi: token.t = 5
let token_id: token.t = 12
module bracket = u8
module stack_change_offset = i16
let stack_change_table_size: i64 = 183
let stack_change_table = [2, 0, 2, 3, 5, 7, 9, 11, 13, 14, 9, 11, 13, 2, 0, 8, 9, 17, 2, 3, 19, 13, 2, 3, 5, 7, 9, 11, 13, 2, 3, 5, 7, 9, 11, 13, 18, 1, 3, 8, 6, 4, 2, 3, 19, 13, 16, 2, 3, 19, 13, 2, 3, 19, 13, 2, 3, 1, 3, 12, 7, 9, 2, 3, 5, 7, 9, 2, 3, 19, 13, 8, 9, 17, 2, 3, 5, 7, 9, 2, 3, 1, 3, 8, 6, 7, 15, 12, 7, 9, 11, 13, 20, 21, 19, 8, 6, 18, 1, 3, 12, 7, 9, 11, 13, 12, 7, 9, 11, 13, 20, 22, 14, 9, 8, 6, 4, 8, 6, 7, 15, 12, 7, 9, 2, 0, 14, 9, 16, 16, 11, 13, 8, 9, 17, 2, 3, 5, 7, 9, 23, 21, 19, 12, 7, 9, 8, 9, 17, 8, 6, 10, 2, 3, 1, 3, 8, 6, 10, 14, 9, 11, 13, 2, 3, 19, 13, 16, 11, 13, 8, 6, 7, 15, 8, 6, 18, 1, 3, 8, 6, 7, 15] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (117, 0), (140, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (36, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (121, 3), (100, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (59, 3), (105, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (18, 4), (51, 4), (-1, -1), (152, 4), (124, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (62, 5), (22, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (47, 4), (163, 4), (-1, -1), (55, 4), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (135, 5), (29, 7), (-1, -1)],
    [(-1, -1), (110, 2), (92, 3), (42, 4), (67, 4), (-1, -1), (79, 4), (13, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (74, 5), (2, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (112, 2), (159, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (126, 2), (9, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (46, 1), (129, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (128, 1), (167, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (39, 3), (174, 5), (-1, -1), (117, 4), (83, 4), (132, 3), (71, 3), (-1, -1), (-1, -1), (156, 3)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (143, 3), (87, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (114, 3), (95, 5), (-1, -1), (179, 4), (170, 4), (15, 3), (146, 3), (-1, -1), (-1, -1), (149, 3)]
] :> [num_tokens][num_tokens](i16, i16)
module parse_offset = i16
let parse_table_size: i64 = 131
let parse_table = [production_stat_list_end, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod, production_atom_paren, production_stat_list_end, production_prod_mul, production_stat_list, production_stat_while, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_compound_stat, production_prod_end, production_sum_end, production_stat_list, production_stat_while, production_atom_variable, production_stat_list, production_stat_while, production_stat_list, production_stat_if, production_stat_list, production_stat_compound, production_compound_stat, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_if, production_prod_div, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_prod_end, production_sum_sub, production_expr, production_sum, production_prod, production_atom_paren, production_program, production_fn_decl, production_prod_end, production_sum_end, production_compound_stat, production_expr, production_sum, production_prod, production_atom_paren, production_expr, production_sum, production_prod, production_atom_paren, production_program_end, production_prod, production_atom_variable, production_prod_end, production_sum_end, production_start, production_program_end, production_prod_end, production_sum_add, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list_end, production_prod, production_atom_variable, production_atom_variable, production_atom_paren, production_prod_mul, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_start, production_program, production_fn_decl, production_expr, production_sum, production_prod, production_atom_variable, production_prod_div, production_prod_end, production_sum_end, production_stat_list, production_stat_compound, production_compound_stat, production_prod_end, production_sum_end, production_prod, production_atom_paren, production_stat_list, production_stat_if, production_atom_paren, production_prod_end, production_sum_sub, production_prod_end, production_sum_end, production_compound_stat, production_prod_end, production_sum_add] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (84, 2), (104, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (25, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (88, 4), (71, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (38, 4), (75, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (11, 2), (33, 2), (-1, -1), (114, 3), (92, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (42, 6), (13, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (31, 2), (121, 2), (-1, -1), (35, 3), (0, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (98, 6), (19, 6), (-1, -1)],
    [(-1, -1), (79, 1), (66, 2), (28, 2), (48, 2), (-1, -1), (57, 3), (9, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (51, 6), (1, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (80, 2), (119, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (93, 2), (7, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (30, 1), (96, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (95, 1), (123, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (26, 2), (126, 3), (-1, -1), (86, 2), (60, 2), (97, 1), (50, 1), (-1, -1), (-1, -1), (117, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (107, 4), (62, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (82, 2), (68, 3), (-1, -1), (129, 2), (124, 2), (10, 1), (111, 1), (-1, -1), (-1, -1), (112, 2)]
] :> [num_tokens][num_tokens](i16, i16)
