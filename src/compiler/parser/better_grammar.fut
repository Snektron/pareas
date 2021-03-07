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
let stack_change_table = [0, 0, 2, 3, 5, 7, 0, 9, 7, 2, 3, 11, 13, 15, 2, 3, 5, 7, 2, 3, 17, 3, 14, 12, 8, 2, 3, 17, 3, 2, 3, 11, 13, 15, 2, 16, 6, 13, 15, 14, 12, 13, 19, 2, 3, 5, 7, 18, 15, 9, 7, 2, 16, 2, 3, 11, 13, 15, 9, 7, 18, 15, 9, 7, 2, 3, 11, 13, 15, 9, 7, 4, 17, 3, 14, 12, 10, 2, 3, 5, 7, 20, 21, 5, 18, 15, 18, 15, 14, 15, 1, 14, 12, 8, 14, 15, 1, 14, 12, 13, 19, 6, 13, 15, 9, 7, 20, 22, 14, 12, 13, 19, 2, 3, 11, 13, 15, 14, 12, 10, 14, 12, 4, 17, 3, 2, 3, 17, 3, 6, 13, 15, 2, 3, 11, 13, 15, 9, 7, 2, 3, 5, 7, 2, 3, 5, 7, 14, 15, 1, 14, 12, 4, 17, 3, 0, 9, 7, 14, 12, 13, 19, 6, 13, 15, 9, 7, 14, 15, 1, 2, 16, 6, 13, 15, 9, 7, 23, 21, 5, 6, 13, 15] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (117, 0), (177, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (71, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (36, 3), (162, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (129, 3), (101, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (139, 4), (14, 4), (-1, -1), (18, 4), (34, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (112, 5), (132, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (77, 4), (43, 4), (-1, -1), (125, 4), (51, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (9, 5), (64, 7), (-1, -1)],
    [(-1, -1), (106, 2), (81, 3), (2, 4), (143, 4), (-1, -1), (25, 4), (170, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (29, 5), (53, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (86, 2), (47, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (84, 2), (60, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (1, 1), (6, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 1), (155, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (74, 3), (150, 5), (-1, -1), (39, 4), (97, 4), (94, 3), (147, 3), (-1, -1), (-1, -1), (22, 3)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (180, 3), (172, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (117, 3), (120, 5), (-1, -1), (108, 4), (158, 4), (167, 3), (88, 3), (-1, -1), (-1, -1), (91, 3)]
] :> [num_tokens][num_tokens](i16, i16)
module parse_offset = i16
let parse_table_size: i64 = 131
let parse_table = [production_atom_variable, production_atom_variable, production_stat_list, production_stat_while, production_atom_paren, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_if, production_stat_list, production_stat_compound, production_compound_stat, production_prod_end, production_sum_end, production_stat_list, production_stat_compound, production_compound_stat, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list_end, production_expr, production_sum, production_prod, production_atom_variable, production_prod_end, production_sum_add, production_stat_list, production_stat_if, production_prod, production_atom_paren, production_stat_list_end, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod, production_atom_paren, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_compound_stat, production_prod_end, production_sum_end, production_stat_list, production_stat_while, production_program, production_fn_decl, production_prod, production_atom_variable, production_prod, production_atom_variable, production_prod_div, production_prod_end, production_sum_end, production_prod_mul, production_prod_end, production_sum_sub, production_expr, production_sum, production_prod, production_atom_paren, production_program_end, production_prod_end, production_sum_add, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_start, production_program_end, production_prod_end, production_sum_end, production_prod_end, production_sum_end, production_compound_stat, production_stat_list, production_stat_compound, production_compound_stat, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list, production_stat_while, production_stat_list, production_stat_if, production_prod_div, production_prod_end, production_sum_end, production_compound_stat, production_atom_paren, production_prod_end, production_sum_sub, production_expr, production_sum, production_prod, production_atom_paren, production_prod_mul, production_stat_list_end, production_expr, production_sum, production_prod, production_atom_paren, production_start, production_program, production_fn_decl, production_expr, production_sum, production_prod, production_atom_variable] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (83, 2), (124, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (53, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (28, 4), (114, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (93, 4), (70, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (103, 2), (11, 2), (-1, -1), (13, 3), (27, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (77, 6), (97, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (56, 2), (34, 2), (-1, -1), (90, 3), (38, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (5, 6), (47, 6), (-1, -1)],
    [(-1, -1), (74, 1), (58, 2), (2, 2), (105, 2), (-1, -1), (18, 3), (119, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (21, 6), (39, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (62, 2), (36, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (60, 2), (45, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (1, 1), (4, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 1), (111, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (54, 2), (108, 3), (-1, -1), (32, 2), (68, 2), (67, 1), (107, 1), (-1, -1), (-1, -1), (16, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (127, 4), (120, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (85, 2), (87, 3), (-1, -1), (75, 2), (112, 2), (118, 1), (64, 1), (-1, -1), (-1, -1), (65, 2)]
] :> [num_tokens][num_tokens](i16, i16)
