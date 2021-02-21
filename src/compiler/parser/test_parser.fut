import "parser"
module test_grammar = import "test_grammar"
module test_parser = parser test_grammar

let main [n] (raw_input: [n]u8) =
    let input: [n]test_grammar.token.t =
        map
            (\b -> match b
                case '(' -> test_grammar.token_soi
                case ')' -> test_grammar.token_eoi
                case '+' -> test_grammar.token_plus
                case 'a' -> test_grammar.token_a
                case '[' -> test_grammar.token_lbracket
                case ']' -> test_grammar.token_rbracket
                case _ -> test_grammar.token_a)
        raw_input
    in test_parser.check input
