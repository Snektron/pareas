import "util"
import "../../../gen/pareas_grammar"
import "../../../lib/github.com/diku-dk/sorts/radix_sort"
module lexer = import "../lexer/lexer"

local type~ lex_table [n] = lexer.lex_table [n] token.t

local let parse_ints [n] (input: []u8) (tokens: [n](token.t, i32, i32)): [n]u32 =
    let parse_int offset len =
        -- The lexer guarantees that integers only consist of one or more numbers, so we don't need to check here.
        -- TODO: Overflow can still occur.
        loop value = 0u32 for i < len do
            let c = u32.u8 input[offset + i]
            in value * 10 - '0' + c

    let (_, offsets, lengths) = unzip3 tokens
    -- Since ints are usually quite short (a few 10s of characters at most) we can simply do a
    -- naive while loop for each of them.
    in map2 parse_int offsets lengths

local let parse_floats [n] (input: []u8) (tokens: [n](token.t, i32, i32)): [n]u32 =
    let parse_float offset len =
        -- The lexer guarantees that floats consist of a sequence of one or more numbers, a dot,
        -- and another sequence of one or more numbers.
        -- This float parsing routine isn't perfect, but it'll do for now.
        let (value, significant) =
            loop (value, significant) = (0f32, 0f32) for i < len do
                let c = input[offset + i]
                in if c == '.'
                    then (value, 1)
                    else (value * 10 - '0' + f32.u8 c, significant * 10)
        in value / significant
    let (_, offsets, lengths) = unzip3 tokens
    in
        map2 parse_float offsets lengths
        |> map f32.to_bits

local let string_link [n] (input: []u8) (tokens: [n](token.t, i32, i32)): [n]i32 =
    let (_, offsets, lengths) = unzip3 tokens
    let bits_per_char = 5 -- a-z + A-Z + 0-9 + _ is 63, so 5 bits will do.
    let get_name_char_bit (bit: i32) (c: u8): i32 =
        let value =
            if c >= 'a' && c <= 'z' then c - 'a'
            else if c >= 'A' && c <= 'Z' then c - 'A' + ('z' - 'a' + 1)
            else if c >= '0' && c <= '0' then c - '0' + ('z' - 'a' + 1) + ('Z' - 'A' + 1)
            else c - '_' + ('z' - 'a' + 1) + ('Z' - 'A' + 1) + ('9' - '0' + 1)
        in u8.get_bit bit value
    let get_name_bit (bit: i32) (index: i32): i32 =
        let bit_in_char = bit % bits_per_char
        let byte_in_string = bit / bits_per_char
        in
            if byte_in_string >= lengths[index] then 0
            else get_name_char_bit bit_in_char input[offsets[index] + byte_in_string]
    -- Expect strings to be roughly equal in length (up to a few 10s of chars), so iterating linearly seems justified
    let str_eq (a: i32) (b: i32) =
        let off_a = i64.i32 offsets[a]
        let len_a = i64.i32 lengths[a]
        let off_b = i64.i32 offsets[b]
        let len_b = i64.i32 lengths[b]
        in if len_a != len_b then false else
        let str_a = (input[off_a : off_a + len_a]) :> [len_a]u8
        let str_b = (input[off_b : off_b + len_a]) :> [len_a]u8
        in map2 (==) str_a str_b |> reduce (&&) true
    let longest = reduce i32.max 0 lengths
    let total_sort_bits = longest * bits_per_char
    let order =
        iota n
        |> map i32.i64
        |> radix_sort total_sort_bits get_name_bit
    let vs: [n]i32 =
        iota n
        |> map (\i -> if i == 0 then false else str_eq order[i] order[i - 1])
        |> map (!)
        |> map i32.bool
        |> scan (+) 0
        |> map (+ -1)
    in scatter
        (replicate n 0i32)
        (map i64.i32 order)
        vs

local let get_token_data (input: []u8) (tokens: [](token.t, i32, i32)) =
    -- Filter out unused tokens
    let tokens: [](token.t, i32, i32) =
        tokens
        |> filter (\(t, _, _) -> t == token_int_literal || t == token_float_literal || t == token_id)
    -- Partition into different types.
    -- TODO: merge with above step?
    let (int_tokens, float_tokens, id_tokens) =
        tokens
        |> partition2
            (\(t, _, _) -> t == token_int_literal)
            (\(t, _, _) -> t == token_float_literal)
    let _ = parse_ints input int_tokens
    let _ = parse_floats input float_tokens
    let _ = string_link input id_tokens
    in false

-- | This pass lexes the input file and produces a list of tokens (which are to be
-- fed into the parser), and (TODO) also produces a list of extra token data, which is to be
-- associated with nodes at a later moment.
let tokenize (input: []u8) (lt: lex_table []) =
    let tokens = lexer.lex input lt
    let parse_token_types =
        tokens
        |> map (.0)
        |> filter (\t -> t != token_whitespace && t != token_comment && t != token_binary_minus_whitespace)
    let _ = get_token_data input tokens
    in parse_token_types

entry main =
    let str = "1234.567"
    let floats = parse_floats str [(0u8, 0i32, i32.i64 (length str))]
    in f32.from_bits floats[0]
