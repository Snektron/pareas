import "parser/parser"
import "passes/util"

module lexer = import "lexer/lexer"
module g = import "../../gen/pareas_grammar"
module pareas_parser = parser g

module pass01 = import "passes/clean_lists"

type~ lex_table [n] = lexer.lex_table [n] g.token.t
type~ stack_change_table [n] = pareas_parser.stack_change_table [n]
type~ parse_table [n] = pareas_parser.parse_table [n]
type~ arity_array = pareas_parser.arity_array

entry mk_lex_table [n] (is: [256]lexer.state) (mt: [n][n]lexer.state) (fs: [n]g.token.t): lex_table [n]
    = lexer.mk_lex_table is mt fs g.identity_state

entry mk_stack_change_table [n]
    (table: [n]g.bracket.t)
    (offsets: [g.num_tokens][g.num_tokens]i32)
    (lengths: [g.num_tokens][g.num_tokens]i32): stack_change_table [n]
    = mk_strtab table offsets lengths

entry mk_parse_table [n]
    (table: [n]g.production.t)
    (offsets: [g.num_tokens][g.num_tokens]i32)
    (lengths: [g.num_tokens][g.num_tokens]i32): parse_table [n]
    = mk_strtab table offsets lengths

entry main [n] [m] [k] [l]
    (input: [n]u8)
    (lt: lex_table [m])
    (sct: stack_change_table [l])
    (pt: parse_table [k])
    (arities: arity_array)
    =
    let (tokens, _, _) =
        lexer.lex input lt
        |> filter (\(t, _, _) -> t != g.token_whitespace && t != g.token_comment && t != g.token_binary_minus_whitespace)
        |> unzip3
    -- As the lexer returns an `invalid` token when the input cannot be lexed, which is accepted
    -- by the parser also, pareas_parser.check will fail whenever there is a lexing error.
    in if !(pareas_parser.check tokens sct) then ([], []) else
    let nodes = pareas_parser.parse tokens pt
    let parents = pareas_parser.build_parent_vector nodes arities
    let (nodes, parents) = pass01.clean_up_lists nodes parents
    in (nodes, parents)
