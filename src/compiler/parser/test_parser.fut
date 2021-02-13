import "parser"
module test_grammar = import "test_grammar"
module test_parser = parser test_grammar

let main [n] (raw_input: [n]u8) =
    let input: [n]test_grammar.token =
        map
            (\b -> match b
                case '(' -> #soi
                case ')' -> #eoi
                case '+' -> #plus
                case 'a' -> #a
                case '[' -> #lbracket
                case ']' -> #rbracket
                case _ -> #a)
        raw_input
    in test_parser.check input
