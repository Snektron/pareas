module frontend = import "frontend"
module backend = import "backend"
module bridge = import "bridge"

import "datatypes"

-- frontend

module g = frontend.g
module production = g.production
module token = g.token

type~ lex_table [n] = frontend.lex_table [n]
type~ stack_change_table [n] = frontend.stack_change_table [n]
type~ parse_table [n] = frontend.parse_table [n]
type~ arity_array = frontend.arity_array

type token = frontend.token

entry mk_lex_table [n] (is: [256]frontend.lexer.state) (mt: [n][n]frontend.lexer.state) (fs: [n]token.t): lex_table [n]
    = frontend.mk_lex_table is mt fs

entry mk_stack_change_table [n]
    (table: [n]g.bracket.t)
    (offsets: [g.num_tokens][g.num_tokens]i32)
    (lengths: [g.num_tokens][g.num_tokens]i32): stack_change_table [n]
    = frontend.mk_stack_change_table table offsets lengths

entry mk_parse_table [n]
    (table: [n]production.t)
    (offsets: [g.num_tokens][g.num_tokens]i32)
    (lengths: [g.num_tokens][g.num_tokens]i32): parse_table [n]
    = frontend.mk_parse_table table offsets lengths

entry frontend_tokenize (input: []u8) (lt: lex_table []): []token =
    frontend.tokenize input lt

entry frontend_num_tokens [n] (_: [n]token): i32 = i32.i64 n

entry frontend_parse (tokens: []token) (sct: stack_change_table []) (pt: parse_table []): (bool, []production.t) =
    frontend.parse tokens sct pt

entry frontend_build_parse_tree [n] (node_types: [n]production.t) (arities: arity_array): [n]i32 =
    frontend.build_parse_tree node_types arities

entry frontend_fix_bin_ops [n] (node_types: *[n]production.t) (parents: *[n]i32): ([]production.t, []i32) =
    frontend.fix_bin_ops node_types parents

entry frontend_fix_if_else [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    frontend.fix_if_else node_types parents

entry frontend_flatten_lists [n] (node_types: *[n]production.t) (parents: *[n]i32): ([n]production.t, [n]i32) =
    frontend.flatten_lists node_types parents

entry frontend_fix_names [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    frontend.fix_names node_types parents

entry frontend_fix_ascriptions [n] (node_types: [n]production.t) (parents: *[n]i32): [n]i32 =
    frontend.fix_ascriptions node_types parents

entry frontend_fix_fn_decls [n] (node_types: [n]production.t) (parents: *[n]i32): (bool, [n]i32) =
    frontend.fix_fn_decls node_types parents

entry frontend_fix_args_and_params [n] (node_types: *[n]production.t) (parents: [n]i32): [n]production.t =
    frontend.fix_args_and_params node_types parents

entry frontend_fix_decls [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    frontend.fix_decls node_types parents

entry frontend_remove_marker_nodes [n] (node_types: [n]production.t) (parents: *[n]i32): [n]i32 =
    frontend.remove_marker_nodes node_types parents

entry frontend_compute_prev_sibling [n] (node_types: *[n]production.t) (parents: *[n]i32): ([]production.t, []i32, []i32) =
    frontend.compute_prev_sibling node_types parents

entry frontend_check_assignments [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32): bool =
    frontend.check_assignments node_types parents prev_siblings

entry frontend_insert_derefs [n] (node_types: *[n]production.t) (parents: *[n]i32) (prev_siblings: *[n]i32): ([]production.t, []i32, []i32) =
    frontend.insert_derefs node_types parents prev_siblings

entry frontend_extract_lexemes [n] (input: []u8) (tokens: []token) (node_types: [n]production.t): [n]u32 =
    frontend.extract_lexemes input tokens node_types

entry frontend_resolve_vars [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data: [n]u32): (bool, [n]i32) =
    frontend.resolve_vars node_types parents prev_siblings data

entry frontend_resolve_fns [n] (node_types: [n]production.t) (resolution: *[n]i32) (data: [n]u32): (bool, [n]i32) =
    frontend.resolve_fns node_types resolution data

entry frontend_resolve_args [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (resolution: *[n]i32): (bool, [n]i32) =
    frontend.resolve_args node_types parents prev_siblings resolution

entry frontend_resolve_data_types [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (resolution: [n]i32): (bool, [n]data_type.t) =
    frontend.resolve_data_types node_types parents prev_siblings resolution

entry frontend_check_return_types [n] (node_types: [n]production.t) (parents: [n]i32) (data_types: [n]data_type): bool =
    frontend.check_return_types node_types parents data_types

entry frontend_check_convergence [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data_types: [n]data_type): bool =
    frontend.check_convergence node_types parents prev_siblings data_types

entry frontend_build_ast [n]
    (node_types: *[n]production.t)
    (parents: *[n]i32)
    (data: *[n]u32)
    (data_types: *[n]data_type)
    (prev_siblings: *[n]i32)
    (resolution: *[n]i32)
    : ([]production.t, []i32, []u32, []data_type, []i32, []i32, []u32)
    = frontend.build_ast node_types parents data data_types prev_siblings resolution

-- backend

type Tree [n] = backend.Tree [n]
type FuncInfo = backend.FuncInfo
type Instr = backend.Instr

entry backend_convert_tree [n]
    (node_types: *[n]production.t)
    (parents: *[n]i32)
    (data: *[n]u32)
    (data_types: *[n]data_type)
    (depths: *[n]i32)
    (child_idx: *[n]i32): Tree[n]
    = bridge.convert_ast node_types data_types parents depths child_idx data

entry backend_preprocess [n] (tree: Tree[n]): (Tree[n]) =
    backend.stage_preprocess tree

entry backend_instr_count [n] (tree: Tree[n]): [n]u32 =
    backend.stage_instr_count tree

entry backend_instr_count_make_function_table [n] (tree: Tree[n]) (instr_offset: [n]u32) =
    backend.stage_instr_count_make_function_table tree instr_offset

entry backend_compact_functab [n] (func_id: [n]u32) (func_start: [n]u32) (func_size: [n]u32): [n]FuncInfo =
    backend.stage_compact_functab func_id func_start func_size

entry backend_instr_gen [n] [k] (tree: Tree[n]) (instr_offset: [n]u32) (func_tab: [k]FuncInfo): []Instr =
    backend.stage_instr_gen tree instr_offset func_tab

entry backend_optimize [n] [m] (instr_data: [n]Instr) (func_tab: [m]FuncInfo): ([n]Instr, [m]FuncInfo, [n]bool) =
    backend.stage_optimize instr_data func_tab

entry backend_regalloc [n] [m] (instrs: [n]Instr) (func_tab: [m]FuncInfo) (func_symbols: [m]u32) (optimize_away: [n]bool): ([]Instr, [m]FuncInfo) =
    backend.stage_regalloc instrs func_tab func_symbols optimize_away

entry backend_fix_jumps [n] [m] (instrs: [n]Instr) (func_tab: [m]FuncInfo): ([]Instr, [m]u32, [m]u32, [m]u32) =
    backend.stage_fix_jumps instrs func_tab

entry backend_postprocess [n] (instrs: [n]Instr) =
    backend.stage_postprocess instrs
