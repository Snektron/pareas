module lexer = import "lexer/lexer"

import "parser/parser"
module g = import "../../gen/pareas_grammar"
local open g
module pareas_parser = parser g

import "util"

import "passes/fix_bin_ops"
import "passes/fix_if_else"
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

entry main [n] [m] [o]
    (input: []u8)
    (lt: lex_table [m])
    (sct: stack_change_table [n])
    (pt: parse_table [o])
    (arities: arity_array)
    : (status_code, []token.t, []i32, []i32)
    =
    let mk_error (code: status_code) = (code, [], [], [])
    let (tokens, _, _) =
        lexer.lex input lt
        |> filter (\(t, _, _) -> t != token_whitespace && t != token_comment && t != token_binary_minus_whitespace)
        |> unzip3
    -- As the lexer returns an `invalid` token when the input cannot be lexed, which is accepted
    -- by the parser also, pareas_parser.check will fail whenever there is a lexing error.
    in if !(pareas_parser.check tokens sct) then mk_error status_parse_error else
    let types = pareas_parser.parse tokens pt
    let parents = pareas_parser.build_parent_vector types arities
    let (types, parents) = fix_bin_ops types parents
    let parents = remove_marker_nodes types parents
    let (valid, types, parents) = fix_if_else types parents
    in if !valid then mk_error status_stray_else_error else
    let (parents, old_old_index) = compactify parents
    let (parents, old_index) = make_preorder_ordering parents
    let types = old_index |> gather old_old_index |> gather types
    in (status_ok, types, parents, old_old_index)
