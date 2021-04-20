module lexer = import "lexer/lexer"

import "parser/parser"
module g = import "../../gen/pareas_grammar"
local open g
module pareas_parser = parser g

import "util"

import "passes/tokenize"
import "passes/fix_bin_ops"
import "passes/fix_if_else"
import "passes/fns_and_assigns"
import "passes/flatten_lists"
import "passes/remove_marker_nodes"
import "passes/compactify"
import "passes/reorder"
import "passes/symbol_resolution"
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
let status_invalid_params: status_code = 3
let status_invalid_assign: status_code = 4
let status_invalid_fn_proto: status_code = 5
let status_duplicate_fn_or_invalid_call: status_code = 6
let status_invalid_variable: status_code = 7

entry main
    (input: []u8)
    (lt: lex_table [])
    (sct: stack_change_table [])
    (pt: parse_table [])
    (arities: arity_array)
    : (status_code, []token.t, []i32, []u32)
    =
    let mk_error (code: status_code) = (code, [], [], [])
    let tokens = tokenize input lt
    let token_types = map (.0) tokens
    -- As the lexer returns an `invalid` token when the input cannot be lexed, which is accepted
    -- by the parser also, pareas_parser.check will fail whenever there is a lexing error.
    in if !(pareas_parser.check token_types sct) then mk_error status_parse_error
    else
    let types = pareas_parser.parse token_types pt
    let parents = pareas_parser.build_parent_vector types arities
    let (types, parents) = fix_bin_ops types parents
    let (parents, old_index) = compactify parents |> unzip
    let types = gather types old_index
    let (valid, types, parents) = fix_if_else types parents
    in if !valid then mk_error status_stray_else_error
    else
    let (types, parents) = fix_fn_args types parents
    let (types, parents) = squish_binds types parents
    let (types, parents) = flatten_lists types parents
    in if !(check_fn_params types parents) then mk_error status_invalid_params
    else
    let parents = remove_marker_nodes types parents
    let (parents, old_index) = compactify parents |> unzip
    let types = gather types old_index
    let depths = compute_depths parents
    let prev_siblings = build_sibling_vector parents depths
    let (types, parents, prev_siblings) = insert_derefs types parents prev_siblings |> unzip3
    -- Note: depths invalid from here.
    in if !(check_fn_decls types parents prev_siblings) then mk_error status_invalid_fn_proto
    else if !(check_assignments types parents prev_siblings) then mk_error status_invalid_assign
    else
    -- ints/floats/names order should be unchanged, relatively, so this is fine.
    let data = build_data_vector types input tokens
    let right_leafs = build_right_leaf_vector parents prev_siblings
    let (vars_valid, data) = resolve_vars types parents prev_siblings right_leafs data
    in if !vars_valid then mk_error status_invalid_variable
    else
    let (calls_valid, data) = resolve_fns types parents data
    in if !calls_valid then mk_error status_duplicate_fn_or_invalid_call
    else
    let left_leafs = build_left_leaf_vector parents prev_siblings
    let (parents, old_index) = build_postorder_ordering parents prev_siblings left_leafs
    -- Note: prev_siblings, right_leafs and left_leafs invalid from here.
    let types = gather types old_index
    let data = gather data old_index
    in (status_ok, types, parents, data)
