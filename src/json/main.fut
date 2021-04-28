module lexer = import "../compiler/lexer/lexer"
import "../compiler/parser/parser"

module g = import "../../gen/json_grammar"
local open g

module json_parser = parser g

type~ lex_table [n] = lexer.lex_table [n] token.t
type~ stack_change_table [n] = json_parser.stack_change_table [n]
type~ parse_table [n] = json_parser.parse_table [n]
type~ arity_array = json_parser.arity_array

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

entry main
    (input: []u8)
    (lt: lex_table [])
    (sct: stack_change_table [])
    (pt: parse_table [])
    (arities: []i32)
    : bool
    =
    let tokens =
        lexer.lex input lt
        |> filter (\(t, _, _) ->  t != token_whitespace)
    let token_types = map (.0) tokens
    in json_parser.check token_types sct
