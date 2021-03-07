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
let stack_change_table = [0, 1, 3, 5, 7, 9, 11, 12, 15, 1, 6, 4, 2, 0, 1, 13, 11, 0, 1, 13, 11, 16, 0, 1, 13, 11, 0, 1, 3, 5, 7, 9, 11, 0, 14, 6, 7, 17, 0, 1, 3, 5, 7, 0, 1, 15, 1, 0, 14, 10, 5, 7, 6, 4, 5, 19, 0, 1, 15, 1, 6, 4, 8, 6, 4, 2, 6, 4, 5, 19, 20, 21, 13, 0, 1, 13, 11, 18, 7, 9, 11, 10, 5, 7, 9, 11, 0, 1, 3, 5, 7, 16, 16, 9, 11, 6, 7, 17, 6, 7, 17, 6, 4, 8, 10, 5, 7, 9, 11, 18, 7, 18, 7, 0, 1, 3, 5, 7, 0, 1, 13, 11, 6, 4, 12, 15, 1, 0, 1, 15, 1, 10, 5, 7, 10, 5, 7, 23, 21, 13, 6, 4, 5, 19, 16, 9, 11, 6, 4, 12, 15, 1, 0, 1, 3, 5, 7, 9, 11, 18, 7, 9, 11, 0, 14, 10, 5, 7, 9, 11, 6, 4, 5, 19, 20, 22, 0, 1, 13, 11, 6, 7, 17] :> [stack_change_table_size]u8
let stack_change_refs = [
    [(-1, -1), (63, 0), (137, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (7, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (49, 3), (81, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (131, 3), (165, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (22, 4), (13, 4), (-1, -1), (56, 4), (47, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (113, 5), (26, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (118, 4), (73, 4), (-1, -1), (127, 4), (163, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (86, 5), (0, 7), (-1, -1)],
    [(-1, -1), (174, 2), (70, 3), (17, 4), (176, 4), (-1, -1), (43, 4), (33, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (38, 5), (152, 7), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (111, 2), (77, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (109, 2), (159, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (21, 1), (92, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (91, 1), (144, 3), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (10, 3), (147, 5), (-1, -1), (52, 4), (170, 4), (95, 3), (180, 3), (-1, -1), (-1, -1), (60, 3)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (134, 3), (104, 5), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (63, 3), (122, 5), (-1, -1), (66, 4), (140, 4), (35, 3), (98, 3), (-1, -1), (-1, -1), (101, 3)]
] :> [num_tokens][num_tokens](i16, i16)
module parse_offset = i16
let parse_table_size: i64 = 131
let parse_table = [production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_compound_stat, production_prod_end, production_sum_end, production_stat_list, production_stat_if, production_stat_list, production_stat_while, production_atom_variable, production_stat_list, production_stat_while, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list_end, production_prod_mul, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_compound, production_compound_stat, production_stat_list_end, production_expr, production_sum, production_prod, production_atom_variable, production_prod_end, production_sum_add, production_stat_list, production_stat_compound, production_compound_stat, production_prod_end, production_sum_end, production_start, production_program_end, production_prod_end, production_sum_end, production_prod_end, production_sum_add, production_program, production_fn_decl, production_stat_list, production_stat_if, production_prod, production_atom_paren, production_expr, production_sum, production_prod, production_atom_paren, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_atom_variable, production_atom_paren, production_prod_mul, production_prod_div, production_prod_end, production_sum_end, production_expr, production_sum, production_prod, production_atom_paren, production_prod, production_atom_variable, production_prod, production_atom_variable, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_variable, production_stat_list, production_stat_while, production_prod_end, production_sum_end, production_compound_stat, production_stat_list, production_stat_compound, production_compound_stat, production_expr, production_sum, production_prod, production_atom_variable, production_expr, production_sum, production_prod, production_atom_variable, production_start, production_program, production_fn_decl, production_prod_end, production_sum_sub, production_atom_paren, production_prod_end, production_sum_end, production_compound_stat, production_stat_list, production_stat_expr, production_expr, production_sum, production_prod, production_atom_paren, production_prod, production_atom_paren, production_stat_list_end, production_expr, production_sum, production_prod, production_atom_paren, production_prod_end, production_sum_sub, production_program_end, production_stat_list, production_stat_if, production_prod_div] :> [parse_table_size]production.t
let parse_refs = [
    [(-1, -1), (45, 2), (103, 3), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (6, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (34, 4), (57, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (95, 4), (121, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (14, 2), (9, 2), (-1, -1), (40, 3), (33, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (81, 6), (16, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (87, 2), (53, 2), (-1, -1), (92, 3), (120, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (61, 6), (0, 6), (-1, -1)],
    [(-1, -1), (127, 1), (51, 2), (11, 2), (128, 2), (-1, -1), (30, 3), (22, 1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (24, 6), (112, 6), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (79, 2), (55, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (77, 2), (118, 2), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (13, 1), (68, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (67, 1), (108, 1), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (7, 2), (109, 3), (-1, -1), (38, 2), (125, 2), (69, 1), (130, 1), (-1, -1), (-1, -1), (43, 2)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (99, 4), (73, 4), (-1, -1)],
    [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (47, 2), (89, 3), (-1, -1), (49, 2), (106, 2), (23, 1), (70, 1), (-1, -1), (-1, -1), (71, 2)]
] :> [num_tokens][num_tokens](i16, i16)
