module production = u8
let num_productions: i64 = 22
let production_atom_paren: production.t = 21
let production_atom_variable: production.t = 20
let production_prod_end: production.t = 19
let production_prod_div: production.t = 18
let production_prod_mul: production.t = 17
let production_sum_end: production.t = 15
let production_sum_sub: production.t = 14
let production_sum_add: production.t = 13
let production_start: production.t = 0
let production_program: production.t = 1
let production_stat_while: production.t = 4
let production_program_end: production.t = 2
let production_stat_expr: production.t = 6
let production_fn_decl: production.t = 3
let production_stat_compound: production.t = 7
let production_stat_if: production.t = 5
let production_stat_list: production.t = 9
let production_prod: production.t = 16
let production_compound_stat: production.t = 8
let production_stat_list_end: production.t = 10
let production_expr: production.t = 11
let production_sum: production.t = 12
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
let stack_change_table = [0, 1, 3, 5, 7, 9, 11, 6, 4, 2, 12, 15, 1, 0, 1, 13, 11, 0, 1, 13, 11, 0, 1, 3, 5, 7, 0, 1, 3, 5, 7, 0, 1, 15, 1, 0, 1, 3, 5, 7, 9, 11, 0, 1, 13, 11, 0, 14, 6, 4, 5, 17, 10, 5, 7, 0, 1, 15, 1, 6, 4, 8, 0, 1, 13, 11, 6, 7, 19, 10, 5, 7, 0, 1, 15, 1, 10, 5, 7, 9, 11, 6, 4, 5, 17, 0, 1, 3, 5, 7, 18, 9, 11, 18, 6, 7, 19, 18, 0, 1, 13, 11, 16, 7, 20, 21, 13, 16, 7, 6, 4, 12, 15, 1, 6, 4, 2, 6, 4, 8, 6, 7, 19, 20, 22, 6, 4, 5, 17, 23, 21, 13, 10, 5, 7, 18, 9, 11, 6, 4, 12, 15, 1, 6, 4, 5, 17, 10, 5, 7, 9, 11, 0, 1, 13, 11, 16, 7, 9, 11, 0, 1, 3, 5, 7, 9, 11, 16, 7, 9, 11, 0, 14, 10, 5, 7, 9, 11, 6, 7, 19, 0, 14] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (114, 0), (129, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (10, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (52, 3), (147, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (69, 3), (76, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (42, 4), (17, 4), (-1, -1), (55, 4), (46, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (21, 5), (35, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (13, 4), (152, 4), (-1, -1), (72, 4), (171, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (85, 5), (0, 7), (-1, -1)],
    [(-1, -1), (123, 2), (104, 3), (98, 4), (62, 4), (-1, -1), (31, 4), (181, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (26, 5), (160, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (102, 2), (156, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (107, 2), (167, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (97, 1), (90, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (93, 1), (135, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (7, 3), (138, 5), (-1, -1), (48, 4), (81, 4), (94, 3), (66, 3), (-1, -1), (-1, -1), (59, 3)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (132, 3), (173, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (114, 3), (109, 5), (-1, -1), (125, 4), (143, 4), (178, 3), (120, 3), (-1, -1), (-1, -1), (117, 3)]
] :> [num_tokens][num_tokens](i16, i16)
module parse_offset = i16
let parse_table_size: i64 = 131
let parse_table = [production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod_end, production_sum_end, production_compound_stat, production_stat_list, production_stat_while, production_stat_list, production_stat_if, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list, production_stat_while, production_stat_list_end, production_prod_end, production_sum_add, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_prod_end, production_sum_end, production_stat_list, production_stat_if, production_prod_div, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_expr, production_sum, production_prod, production_atom_paren, production_prod_end, production_sum_sub, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_atom_paren, production_atom_variable, production_prod_mul, production_atom_variable, production_stat_list, production_stat_while, production_prod, production_atom_variable, production_program, production_fn_decl, production_prod, production_atom_variable, production_prod_end, production_sum_end, production_compound_stat, production_start, production_program_end, production_prod_end, production_sum_end, production_prod_end, production_sum_end, production_prod_div, production_program_end, production_prod_end, production_sum_add, production_start, production_program, production_fn_decl, production_expr, production_sum, production_prod, production_atom_variable, production_atom_paren, production_prod_end, production_sum_end, production_compound_stat, production_prod_end, production_sum_sub, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list, production_stat_if, production_prod, production_atom_paren, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod, production_atom_paren, production_stat_list_end, production_expr, production_sum, production_prod, production_atom_paren, production_prod_mul, production_stat_list_end] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (85, 2), (95, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (8, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (39, 4), (108, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (51, 4), (58, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (34, 2), (11, 2), (-1, -1), (43, 3), (36, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (13, 6), (28, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (9, 2), (112, 2), (-1, -1), (55, 3), (124, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (64, 6), (0, 6), (-1, -1)],
    [(-1, -1), (92, 1), (78, 2), (74, 2), (48, 2), (-1, -1), (25, 3), (130, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (19, 6), (116, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (76, 2), (114, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (80, 2), (122, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (73, 1), (70, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (71, 1), (102, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (6, 2), (103, 3), (-1, -1), (37, 2), (62, 2), (72, 1), (50, 1), (-1, -1), (-1, -1), (46, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (98, 4), (125, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (87, 2), (82, 3), (-1, -1), (93, 2), (106, 2), (129, 1), (91, 1), (-1, -1), (-1, -1), (89, 2)]
] :> [num_tokens][num_tokens](i16, i16)
