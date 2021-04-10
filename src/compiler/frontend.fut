module lexer = import "lexer/lexer"

import "parser/parser"
module g = import "../../gen/pareas_grammar"
local open g
module pareas_parser = parser g

import "util"

import "passes/tokenize"
import "passes/fix_bin_ops"
import "passes/fix_if_else"
import "passes/fn_stuff"
import "passes/remove_marker_nodes"
import "passes/compactify"
import "passes/preorder"

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

entry main
    (input: []u8)
    (lt: lex_table [])
    (sct: stack_change_table [])
    (pt: parse_table [])
    (arities: arity_array)
    : (status_code, []token.t, []i32, []u32, []i32)
    =
    let mk_error (code: status_code) = (code, [], [], [], [])
    let tokens = tokenize input lt
    let token_types = map (.0) tokens
    -- As the lexer returns an `invalid` token when the input cannot be lexed, which is accepted
    -- by the parser also, pareas_parser.check will fail whenever there is a lexing error.
    in if !(pareas_parser.check token_types sct) then mk_error status_parse_error else
    let types = pareas_parser.parse token_types pt
    let parents = pareas_parser.build_parent_vector types arities
    let (types, parents) = fix_bin_ops types parents
    let (parents, old_index) = compactify parents |> unzip
    let types = gather types old_index
    let (valid, types, parents) = fix_if_else types parents
    in if !valid then mk_error status_stray_else_error else
    let (types, parents) = fix_fn_args types parents
    let (types, parents) = squish_binds types parents
    in if !(check_fn_params types parents) then mk_error status_invalid_params else
    let parents = remove_marker_nodes types parents
    let (parents, old_old_index) = compactify parents |> unzip
    let (parents, old_index) = make_preorder_ordering parents
    let types = old_index |> gather old_old_index |> gather types
    -- ints/floats/identifiers should be unchanged, relatively, so this is fine.
    let data = build_data_vector types input tokens
    in (status_ok, types, parents, data, parents)
