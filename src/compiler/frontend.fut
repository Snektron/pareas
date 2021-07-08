module lexer = import "lexer/lexer"

import "parser/parser"
module g = import "../../gen/pareas_grammar"
local open g
module pareas_parser = parser g

import "util"
import "datatypes"

import "passes/tokenize"
import "passes/fix_bin_ops"
import "passes/fix_if_else"
import "passes/compactify"
import "passes/flatten_lists"
import "passes/fns_and_assigns"
import "passes/remove_marker_nodes"
import "passes/reorder"
import "passes/symbol_resolution"
import "passes/type_resolution"
import "passes/check_return_paths"
import "passes/ids"
import "passes/util"

type~ lex_table [n] = lexer.lex_table [n] token.t
type~ stack_change_table [n] = pareas_parser.stack_change_table [n]
type~ parse_table [n] = pareas_parser.parse_table [n]
type~ arity_array = pareas_parser.arity_array

entry mk_lex_table [n] (is: [256]lexer.state) (mt: [n][n]lexer.state) (fs: [n]token.t): lex_table [n]
    = lexer.mk_lex_table is mt fs identity_state

entry mk_stack_change_table [n]
    (table: [n]bracket.t)
    (offsets: [num_tokens][num_tokens]i32)
    (lengths: [num_tokens][num_tokens]i32): stack_change_table [n]
    = mk_strtab table offsets lengths

entry mk_parse_table [n]
    (table: [n]production.t)
    (offsets: [num_tokens][num_tokens]i32)
    (lengths: [num_tokens][num_tokens]i32): parse_table [n]
    = mk_strtab table offsets lengths

-- Keep in sync with src/compiler/main.fut
type status_code = u8
let status_ok: status_code = 0
let status_parse_error: status_code = 1
let status_stray_else_error: status_code = 2
let status_invalid_decl: status_code = 3
let status_invalid_params: status_code = 4
let status_invalid_assign: status_code = 5
let status_invalid_fn_proto: status_code = 6
let status_duplicate_fn_or_invalid_call: status_code = 7
let status_invalid_variable: status_code = 8
let status_invalid_arg_count: status_code = 9
let status_type_error: status_code = 10
let status_invalid_return: status_code = 11
let status_missing_return: status_code = 12

type token = (token.t, i32, i32)
entry frontend_tokenize (input: []u8) (lt: lex_table []): []token =
    tokenize input lt

entry frontend_parse (tokens: []token) (sct: stack_change_table []) (pt: parse_table []): (bool, []production.t) =
    let token_types = map (.0) tokens
    in if pareas_parser.check token_types sct
        then (true, pareas_parser.parse token_types pt)
        else (false, [])

entry frontend_build_parse_tree [n] (node_types: [n]production.t) (arities: arity_array): [n]i32 =
    pareas_parser.build_parent_vector node_types arities

entry frontend_fix_bin_ops [n] (node_types: *[n]production.t) (parents: *[n]i32): ([]production.t, []i32) =
    let (node_types, parents) = fix_bin_ops node_types parents
    let (parents, old_index) = compactify parents |> unzip
    let node_types = gather node_types old_index
    in (node_types, parents)

entry frontend_fix_if_else [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    fix_if_else node_types parents

entry frontend_flatten_lists [n] (node_types: *[n]production.t) (parents: *[n]i32): ([n]production.t, [n]i32) =
    flatten_lists node_types parents

entry frontend_fix_names [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    fix_names node_types parents

entry frontend_fix_ascriptions [n] (node_types: [n]production.t) (parents: *[n]i32): [n]i32 =
    fix_ascriptions node_types parents

entry frontend_fix_fn_decls [n] (node_types: [n]production.t) (parents: *[n]i32): (bool, [n]i32) =
    fix_fn_decls node_types parents

entry frontend_fix_args_and_params [n] (node_types: *[n]production.t) (parents: [n]i32): [n]production.t =
    let node_types = reinsert_arg_lists node_types
    let node_types = fix_param_lists node_types parents
    in node_types

entry frontend_fix_decls [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    let node_types = fix_param_lists node_types parents
    let valid = check_fn_params node_types parents
    let (node_types, parents) = squish_decl_ascripts node_types parents
    in (valid, node_types, parents)

entry frontend_remove_marker_nodes [n] (node_types: [n]production.t) (parents: *[n]i32): [n]i32 =
    remove_marker_nodes node_types parents

entry frontend_compute_prev_sibling [n] (node_types: *[n]production.t) (parents: *[n]i32): ([]production.t, []i32, []i32) =
    let (parents, old_index) = compactify parents |> unzip
    let node_types = gather node_types old_index
    let depths = compute_depths parents
    let prev_siblings = build_sibling_vector parents depths
    in (node_types, parents, prev_siblings)

entry frontend_check_assignments [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32): bool =
    check_assignments node_types parents prev_siblings

entry frontend_insert_derefs [n] (node_types: *[n]production.t) (parents: *[n]i32) (prev_siblings: *[n]i32)
    : ([]production.t, []i32, []i32)
    =
    insert_derefs node_types parents prev_siblings |> unzip3

entry frontend_extract_lexemes [n] (input: []u8) (tokens: []token) (node_types: [n]production.t): [n]u32 =
    build_data_vector node_types input tokens

entry frontend_resolve_vars [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data: [n]u32): (bool, [n]i32) =
    let right_leafs = build_right_leaf_vector parents prev_siblings
    in resolve_vars node_types parents prev_siblings right_leafs data

entry frontend_resolve_fns [n] (node_types: [n]production.t) (resolution: *[n]i32) (data: [n]u32): (bool, [n]i32) =
    let (valid, fn_resolution) = resolve_fns node_types data
    -- This works because declarations and function calls are disjoint.
    let resolution = merge_resolutions resolution fn_resolution
    in (valid, resolution)

entry frontend_resolve_args [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (resolution: *[n]i32): (bool, [n]i32) =
    let (valid, arg_resolution) = resolve_args node_types parents prev_siblings resolution
    -- This works because declarations, function calls, and function arg wrappers are disjoint.
    let resolution = merge_resolutions resolution arg_resolution
    in (valid, resolution)

entry frontend_resolve_data_types [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (resolution: [n]i32): (bool, [n]data_type.t) =
    let data_types = resolve_types node_types parents prev_siblings resolution
    let types_valid = check_types node_types parents prev_siblings data_types
    in (types_valid, data_types)

entry frontend_check_return_types [n] (node_types: [n]production.t) (parents: [n]i32) (data_types: [n]data_type): bool =
    check_return_types node_types parents data_types

entry frontend_check_convergence [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data_types: [n]data_type): bool =
    check_return_paths node_types parents prev_siblings data_types

entry frontend_build_ast [n]
    (node_types: *[n]production.t)
    (parents: *[n]i32)
    (data: *[n]u32)
    (data_types: *[n]data_type)
    (prev_siblings: *[n]i32)
    (resolution: *[n]i32)
    : ([]production.t, []i32, []u32, []data_type, []i32, []i32, []i32)
    =
    let (data, fn_tab) = assign_ids node_types resolution data_types data
    -- Compute the child index from the parent
    let child_indexes = compute_depths prev_siblings
    let left_leafs = build_left_leaf_vector parents prev_siblings
    let (parents, old_index) = build_postorder_ordering parents prev_siblings left_leafs
    -- Note: prev_siblings, right_leafs, left_leafs and resolution invalid from here.
    let node_types = gather node_types old_index
    let data = gather data old_index
    let data_types = gather data_types old_index
    let child_indexes = gather child_indexes old_index
    -- Re-compute the depths
    let depths = compute_depths parents
    in (node_types, parents, data, data_types, depths, child_indexes, fn_tab)

entry main
    (input: []u8)
    (lt: lex_table [])
    (sct: stack_change_table [])
    (pt: parse_table [])
    (arities: arity_array)
    : (status_code, []production.t, []i32, []u32, []data_type, []i32, []i32, []i32)
    =
    let mk_error (code: status_code) = (code, [], [], [], [], [], [], [])
    let tokens = tokenize input lt
    let token_types = map (.0) tokens
    -- As the lexer returns an `invalid` token when the input cannot be lexed, which is accepted
    -- by the parser also, pareas_parser.check will fail whenever there is a lexing error.
    in if !(pareas_parser.check token_types sct) then mk_error status_parse_error
    else
    let node_types = pareas_parser.parse token_types pt
    let parents = pareas_parser.build_parent_vector node_types arities
    let (node_types, parents) = fix_bin_ops node_types parents
    let (parents, old_index) = compactify parents |> unzip
    let node_types = gather node_types old_index
    let (valid, node_types, parents) = fix_if_else node_types parents
    in if !valid then mk_error status_stray_else_error
    else
    let (node_types, parents) = flatten_lists node_types parents
    let (valid, node_types, parents) = fix_names node_types parents
    in if !valid then mk_error status_invalid_decl
    else
    let parents = fix_ascriptions node_types parents
    let (valid, parents) = fix_fn_decls node_types parents
    in if !valid then mk_error status_invalid_fn_proto
    else
    let node_types = reinsert_arg_lists node_types
    let node_types = fix_param_lists node_types parents
    let valid = check_fn_params node_types parents
    -- only check for validity after squish so that futhark can better merge these passes.
    let (node_types, parents) = squish_decl_ascripts node_types parents
    in if !valid then mk_error status_invalid_params
    else
    let parents = remove_marker_nodes node_types parents
    let (parents, old_index) = compactify parents |> unzip
    let node_types = gather node_types old_index
    let depths = compute_depths parents
    let prev_siblings = build_sibling_vector parents depths
    let (node_types, parents, prev_siblings) = insert_derefs node_types parents prev_siblings |> unzip3
    -- Note: depths invalid from here.
    in if !(check_assignments node_types parents prev_siblings) then mk_error status_invalid_assign
    else
    -- ints/floats/names order should be unchanged, relatively, so this is fine.
    let data = build_data_vector node_types input tokens
    let right_leafs = build_right_leaf_vector parents prev_siblings
    let (valid, resolution) = resolve_vars node_types parents prev_siblings right_leafs data
    in if !valid then mk_error status_invalid_variable
    else
    let (valid, fn_resolution) = resolve_fns node_types data
    -- This works because declarations and function calls are disjoint.
    let resolution = merge_resolutions resolution fn_resolution
    in if !valid then mk_error status_duplicate_fn_or_invalid_call
    else
    let (valid, arg_resolution) = resolve_args node_types parents prev_siblings resolution
    -- This works because declarations, function calls, and function arg wrappers are disjoint.
    let resolution = merge_resolutions resolution arg_resolution
    in if !valid then mk_error status_invalid_arg_count
    else
    let data_types = resolve_types node_types parents prev_siblings resolution
    let types_valid = check_types node_types parents prev_siblings data_types
    let returns_valid = check_return_types node_types parents data_types
    in if !types_valid then mk_error status_type_error
    else if !returns_valid then mk_error status_invalid_return
    else
    let paths_valid = check_return_paths node_types parents prev_siblings data_types
    in if !paths_valid then mk_error status_missing_return
    else
    let (data, fn_tab) = assign_ids node_types resolution data_types data
    -- Compute the child index from the parent
    let child_indexes = compute_depths prev_siblings
    let left_leafs = build_left_leaf_vector parents prev_siblings
    let (parents, old_index) = build_postorder_ordering parents prev_siblings left_leafs
    -- Note: prev_siblings, right_leafs, left_leafs and resolution invalid from here.
    let node_types = gather node_types old_index
    let data = gather data old_index
    let data_types = gather data_types old_index
    let child_indexes = gather child_indexes old_index
    -- Re-compute the depths
    let depths = compute_depths parents
    in (status_ok, node_types, parents, data, data_types, depths, child_indexes, fn_tab)
