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

type token = (token.t, i32, i32)
entry tokenize (input: []u8) (lt: lex_table []): []token =
    tokenize input lt

entry num_tokens [n] (_: [n]token): i32 = i32.i64 n

entry parse (tokens: []token) (sct: stack_change_table []) (pt: parse_table []): (bool, []production.t) =
    let token_types = map (.0) tokens
    in if pareas_parser.check token_types sct
        then (true, pareas_parser.parse token_types pt)
        else (false, [])

entry build_parse_tree [n] (node_types: [n]production.t) (arities: arity_array): [n]i32 =
    pareas_parser.build_parent_vector node_types arities

entry fix_bin_ops [n] (node_types: *[n]production.t) (parents: *[n]i32): ([]production.t, []i32) =
    let (node_types, parents) = fix_bin_ops node_types parents
    let (parents, old_index) = compactify parents |> unzip
    let node_types = gather node_types old_index
    in (node_types, parents)

entry fix_if_else [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    fix_if_else node_types parents

entry flatten_lists [n] (node_types: *[n]production.t) (parents: *[n]i32): ([n]production.t, [n]i32) =
    flatten_lists node_types parents

entry fix_names [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    fix_names node_types parents

entry fix_ascriptions [n] (node_types: [n]production.t) (parents: *[n]i32): [n]i32 =
    fix_ascriptions node_types parents

entry fix_fn_decls [n] (node_types: [n]production.t) (parents: *[n]i32): (bool, [n]i32) =
    fix_fn_decls node_types parents

entry fix_args_and_params [n] (node_types: *[n]production.t) (parents: [n]i32): [n]production.t =
    let node_types = reinsert_arg_lists node_types
    let node_types = fix_param_lists node_types parents
    in node_types

entry fix_decls [n] (node_types: *[n]production.t) (parents: *[n]i32): (bool, [n]production.t, [n]i32) =
    let node_types = fix_param_lists node_types parents
    let valid = check_fn_params node_types parents
    let (node_types, parents) = squish_decl_ascripts node_types parents
    in (valid, node_types, parents)

entry remove_marker_nodes [n] (node_types: [n]production.t) (parents: *[n]i32): [n]i32 =
    remove_marker_nodes node_types parents

entry compute_prev_sibling [n] (node_types: *[n]production.t) (parents: *[n]i32): ([]production.t, []i32, []i32) =
    let (parents, old_index) = compactify parents |> unzip
    let node_types = gather node_types old_index
    let depths = compute_depths parents
    let prev_siblings = build_sibling_vector parents depths
    in (node_types, parents, prev_siblings)

entry check_assignments [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32): bool =
    check_assignments node_types parents prev_siblings

entry insert_derefs [n] (node_types: *[n]production.t) (parents: *[n]i32) (prev_siblings: *[n]i32): ([]production.t, []i32, []i32) =
    insert_derefs node_types parents prev_siblings |> unzip3

entry extract_lexemes [n] (input: []u8) (tokens: []token) (node_types: [n]production.t): [n]u32 =
    build_data_vector node_types input tokens

entry resolve_vars [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data: [n]u32): (bool, [n]i32) =
    let right_leafs = build_right_leaf_vector parents prev_siblings
    in resolve_vars node_types parents prev_siblings right_leafs data

entry resolve_fns [n] (node_types: [n]production.t) (resolution: *[n]i32) (data: [n]u32): (bool, [n]i32) =
    let (valid, fn_resolution) = resolve_fns node_types data
    -- This works because declarations and function calls are disjoint.
    let resolution = merge_resolutions resolution fn_resolution
    in (valid, resolution)

entry resolve_args [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (resolution: *[n]i32): (bool, [n]i32) =
    let (valid, arg_resolution) = resolve_args node_types parents prev_siblings resolution
    -- This works because declarations, function calls, and function arg wrappers are disjoint.
    let resolution = merge_resolutions resolution arg_resolution
    in (valid, resolution)

entry resolve_data_types [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (resolution: [n]i32): (bool, [n]data_type.t) =
    let data_types = resolve_types node_types parents prev_siblings resolution
    let types_valid = check_types node_types parents prev_siblings data_types
    in (types_valid, data_types)

entry check_return_types [n] (node_types: [n]production.t) (parents: [n]i32) (data_types: [n]data_type): bool =
    check_return_types node_types parents data_types

entry check_convergence [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data_types: [n]data_type): bool =
    check_return_paths node_types parents prev_siblings data_types

entry build_ast [n]
    (node_types: *[n]production.t)
    (parents: *[n]i32)
    (data: *[n]u32)
    (data_types: *[n]data_type)
    (prev_siblings: *[n]i32)
    (resolution: *[n]i32)
    : ([]production.t, []i32, []u32, []data_type, []i32, []i32, []u32)
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
