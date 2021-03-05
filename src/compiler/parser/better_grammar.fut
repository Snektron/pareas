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
let stack_change_table = [0, 1, 3, 5, 7, 9, 11, 12, 15, 1, 6, 4, 2, 0, 14, 0, 1, 3, 5, 7, 9, 11, 16, 7, 9, 11, 0, 1, 13, 11, 0, 1, 3, 5, 7, 9, 11, 0, 1, 13, 11, 18, 0, 14, 6, 4, 5, 17, 10, 5, 7, 0, 1, 15, 1, 0, 1, 3, 5, 7, 0, 1, 13, 11, 6, 7, 19, 10, 5, 7, 0, 1, 15, 1, 0, 1, 13, 11, 16, 7, 9, 11, 0, 1, 13, 11, 20, 21, 13, 20, 22, 6, 4, 5, 17, 10, 5, 7, 9, 11, 18, 18, 9, 11, 6, 7, 19, 0, 1, 3, 5, 7, 6, 4, 8, 0, 1, 15, 1, 16, 7, 6, 4, 5, 17, 16, 7, 0, 1, 13, 11, 0, 1, 3, 5, 7, 6, 4, 2, 6, 4, 12, 15, 1, 6, 4, 12, 15, 1, 18, 9, 11, 6, 4, 5, 17, 10, 5, 7, 9, 11, 23, 21, 13, 10, 5, 7, 10, 5, 7, 9, 11, 6, 7, 19, 0, 14, 6, 7, 19, 6, 4, 8] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (136, 0), (161, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (7, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (48, 3), (156, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (67, 3), (95, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (26, 4), (82, 4), (-1, -1), (115, 4), (42, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (131, 5), (30, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (127, 4), (74, 4), (-1, -1), (70, 4), (13, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (107, 5), (0, 7), (-1, -1)],
    [(-1, -1), (89, 2), (86, 3), (37, 4), (60, 4), (-1, -1), (51, 4), (175, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (55, 5), (15, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (125, 2), (78, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (119, 2), (22, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (41, 1), (101, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (100, 1), (149, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (10, 3), (144, 5), (-1, -1), (44, 4), (91, 4), (104, 3), (64, 3), (-1, -1), (-1, -1), (112, 3)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (164, 3), (167, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (136, 3), (139, 5), (-1, -1), (121, 4), (152, 4), (172, 3), (177, 3), (-1, -1), (-1, -1), (180, 3)]
] :> [num_tokens][num_tokens](i16, i16)
module parse_offset = i16
let parse_table_size: i64 = 131
let parse_table = [production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_compound_stat, production_prod_end, production_sum_end, production_stat_list_end, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod, production_atom_paren, production_stat_list, production_stat_while, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list, production_stat_while, production_atom_variable, production_stat_list_end, production_prod_end, production_sum_add, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_if, production_prod_div, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_stat_list, production_stat_if, production_prod, production_atom_paren, production_stat_list, production_stat_if, production_program, production_fn_decl, production_program_end, production_prod_end, production_sum_sub, production_expr, production_sum, production_prod, production_atom_paren, production_atom_variable, production_atom_paren, production_prod_mul, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_prod_end, production_sum_end, production_stat_list, production_stat_compound, production_compound_stat, production_prod, production_atom_variable, production_prod_end, production_sum_add, production_prod, production_atom_variable, production_stat_list, production_stat_while, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_start, production_program_end, production_prod_end, production_sum_end, production_prod_end, production_sum_end, production_compound_stat, production_prod_end, production_sum_end, production_compound_stat, production_atom_paren, production_prod_end, production_sum_sub, production_expr, production_sum, production_prod, production_atom_paren, production_start, production_program, production_fn_decl, production_expr, production_sum, production_prod, production_atom_variable, production_expr, production_sum, production_prod, production_atom_paren, production_prod_mul, production_stat_list_end, production_prod_div, production_prod_end, production_sum_end] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (98, 2), (115, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (6, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (32, 4), (111, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (48, 4), (66, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (18, 2), (59, 2), (-1, -1), (81, 3), (29, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (92, 6), (20, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (90, 2), (55, 2), (-1, -1), (52, 3), (9, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (73, 6), (0, 6), (-1, -1)],
    [(-1, -1), (63, 1), (61, 2), (26, 2), (45, 2), (-1, -1), (36, 3), (127, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (39, 6), (10, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (88, 2), (57, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (84, 2), (16, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (28, 1), (71, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (70, 1), (108, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (7, 2), (105, 3), (-1, -1), (30, 2), (64, 2), (72, 1), (47, 1), (-1, -1), (-1, -1), (79, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (118, 4), (122, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (100, 2), (102, 3), (-1, -1), (86, 2), (109, 2), (126, 1), (128, 1), (-1, -1), (-1, -1), (129, 2)]
] :> [num_tokens][num_tokens](i16, i16)
