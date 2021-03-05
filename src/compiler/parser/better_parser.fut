import "parser"
module better_grammar = import "better_grammar"
module better_parser = parser better_grammar

let main [n] (raw_input: [n]u8) =
    let input: [n]better_grammar.token.t =
        map
            (\b -> match b
                case '[' -> better_grammar.special_token_soi
                case ']' -> better_grammar.special_token_eoi
                case 't' -> better_grammar.token_id
                case '*' -> better_grammar.token_star
                case '+' -> better_grammar.token_plus
                case 'f' -> better_grammar.token_fn
                case '-' -> better_grammar.token_minus
                case '(' -> better_grammar.token_lparen
                case ')' -> better_grammar.token_rparen
                case '/' -> better_grammar.token_slash
                case 'w' -> better_grammar.token_while
                case '{' -> better_grammar.token_lbrace
                case 'i' -> better_grammar.token_if
                case ';' -> better_grammar.token_semi
                case '}' -> better_grammar.token_rbrace)
        raw_input
    in if !(better_parser.check input) then -1 else
    let parse = better_parser.parse input
    let parents = better_parser.build_parent_vector parse
    in last parents
