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
let token_soi: token.t = 0
let token_lbrace: token.t = 6
let token_eoi: token.t = 1
let token_fn: token.t = 2
let token_plus: token.t = 8
let token_lparen: token.t = 13
let token_if: token.t = 4
let token_slash: token.t = 11
let token_rbrace: token.t = 7
let token_while: token.t = 3
let token_minus: token.t = 9
let token_star: token.t = 10
let token_semi: token.t = 5
let token_id: token.t = 12
module bracket = u8
module stack_change_offset = i16
let stack_change_table_size: i64 = 183
let stack_change_table = [0, 1, 3, 1, 4, 7, 9, 0, 1, 11, 5, 0, 2, 0, 1, 11, 5, 0, 1, 13, 7, 9, 0, 1, 11, 5, 8, 6, 12, 0, 1, 13, 7, 9, 15, 5, 8, 6, 14, 16, 0, 1, 13, 7, 9, 0, 1, 3, 1, 0, 1, 11, 5, 0, 1, 11, 5, 8, 6, 7, 19, 8, 9, 17, 0, 1, 11, 5, 0, 2, 8, 9, 17, 20, 21, 11, 4, 7, 9, 8, 6, 10, 3, 1, 8, 6, 10, 3, 1, 20, 22, 16, 15, 5, 4, 7, 9, 15, 5, 4, 7, 9, 18, 9, 18, 9, 16, 4, 7, 9, 15, 5, 4, 7, 9, 15, 5, 23, 21, 11, 0, 1, 13, 7, 9, 16, 15, 5, 10, 3, 1, 8, 9, 17, 18, 9, 15, 5, 18, 9, 15, 5, 0, 1, 13, 7, 9, 15, 5, 8, 6, 7, 19, 8, 6, 12, 0, 2, 8, 6, 14, 8, 6, 7, 19, 0, 1, 13, 7, 9, 15, 5, 8, 6, 7, 19, 8, 9, 17, 0, 1, 3, 1] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (89, 0), (117, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (128, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (4, 3), (107, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (99, 3), (94, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (7, 4), (22, 4), (-1, -1), (179, 4), (156, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (17, 5), (165, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (64, 4), (49, 4), (-1, -1), (0, 4), (11, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (40, 5), (29, 7), (-1, -1)],
    [(-1, -1), (89, 2), (73, 3), (53, 4), (13, 4), (-1, -1), (45, 4), (68, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (120, 5), (142, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (104, 2), (134, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (102, 2), (138, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (106, 1), (91, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (39, 1), (125, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (26, 3), (84, 5), (-1, -1), (172, 4), (149, 4), (61, 3), (70, 3), (-1, -1), (-1, -1), (158, 3)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (76, 3), (112, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (153, 3), (79, 5), (-1, -1), (57, 4), (161, 4), (176, 3), (131, 3), (-1, -1), (-1, -1), (36, 3)]
] :> [num_tokens][num_tokens](i16, i16)
module parse_offset = i16
let parse_table_size: i64 = 131
let parse_table = [production_stat_list, production_stat_compound, production_compound_stat, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_while, production_stat_list_end, production_stat_list, production_stat_if, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_if, production_prod_end, production_sum_end, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod_end, production_sum_end, production_atom_variable, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_stat_list, production_stat_if, production_stat_list, production_stat_while, production_prod_end, production_sum_add, production_prod_mul, production_stat_list, production_stat_while, production_stat_list_end, production_prod_div, production_program, production_fn_decl, production_expr, production_sum, production_prod, production_atom_variable, production_prod_end, production_sum_end, production_compound_stat, production_prod_end, production_sum_end, production_compound_stat, production_start, production_program_end, production_program_end, production_atom_paren, production_expr, production_sum, production_prod, production_atom_paren, production_expr, production_sum, production_prod, production_atom_variable, production_prod, production_atom_variable, production_prod, production_atom_variable, production_atom_variable, production_expr, production_sum, production_prod, production_atom_paren, production_expr, production_sum, production_prod, production_atom_paren, production_start, production_program, production_fn_decl, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_atom_paren, production_compound_stat, production_prod_div, production_prod, production_atom_paren, production_prod, production_atom_paren, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod_end, production_sum_sub, production_prod_end, production_sum_end, production_stat_list_end, production_prod_end, production_sum_end, production_prod_end, production_sum_sub, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod_end, production_sum_add, production_prod_mul, production_stat_list, production_stat_compound, production_compound_stat] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (63, 2), (88, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (98, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (3, 4), (80, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (71, 4), (67, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (7, 2), (18, 2), (-1, -1), (128, 3), (114, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (12, 6), (119, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (47, 2), (40, 2), (-1, -1), (0, 3), (9, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (31, 6), (22, 6), (-1, -1)],
    [(-1, -1), (65, 1), (51, 2), (42, 2), (10, 2), (-1, -1), (37, 3), (49, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (91, 6), (104, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (77, 2), (100, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (75, 2), (102, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (79, 1), (66, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (30, 1), (97, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (20, 2), (60, 3), (-1, -1), (125, 2), (110, 2), (46, 1), (50, 1), (-1, -1), (-1, -1), (115, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (53, 4), (84, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (112, 2), (57, 3), (-1, -1), (44, 2), (117, 2), (127, 1), (99, 1), (-1, -1), (-1, -1), (28, 2)]
] :> [num_tokens][num_tokens](i16, i16)
