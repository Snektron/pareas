import "parser"
module test_parser = parser (import "test_grammar")

let main = test_parser.check [#soi, #a, #plus, #lbracket, #a, #plus, #a, #rbracket, #eoi]
