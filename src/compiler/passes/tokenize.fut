import "util"
import "../util"
import "../../../gen/pareas_grammar"
import "../../../lib/github.com/diku-dk/sorts/radix_sort"
module lexer = import "../lexer/lexer"

-- Some useful typedefs so that these don't need to be kindped out ever type, cluttering the code.
local type~ lex_table [n] = lexer.lex_table [n] token.t
local type tokenref = (token.t, i32, i32)

-- | Parse an integer literal token into an u32. Overflow is not handled.
-- As integers are not supposed to be very long, the parsing of each itneger is simply done
-- as a simple loop.
-- As the lexer guarantees that integer literals are a simple sequence of digits, there is no
-- check required whether all the characters are integers.
-- TODO: As an optimization case for very long integers, overflow could be checked and failure
-- could be returned. This would also yield a static upper bound for the number of loop iterations.
local let parse_int (input: []u8) ((_, offset, len): tokenref): u32 =
    loop value = 0u32 for i < len do
        let c = u32.u8 input[offset + i]
        in value * 10 - '0' + c

-- | Parse a float literal token into a f32. Overflow is not handled, and if literals
-- are longer than the 8 digits of accuracy floats provide, the parsed literal value
-- may not be accurate.
-- Again, float literals should not be very long, so they are parsed simply using a linear loop.
-- As the lexer guarantees that floats consists of one or more digits, a dot, then another one or
-- mopre digits, this does not need to handle checking whether the float's format is valid.
-- TODO: For an optimization case, we could check whether a float is (much) longer than the 8
-- digits of accuracy the floating point spec manditates.
local let parse_float (input: []u8) ((_, offset, len): tokenref): f32 =
    let (value, significant) =
        loop (value, significant) = (0f32, 0f32) for i < len do
                let c = input[offset + i]
                in if c == '.'
                    then (value, 1)
                    else (value * 10 - '0' + f32.u8 c, significant * 10)
    in value / significant

-- | Given a list of name tokens, assign a unique ID to every unique name. This replaces the need
-- for annoying string operations further in the compiler, and allows us to simply query and compare the IDs.
-- For now, this implementation does a rather simply fixed-length radix sort, as names are not supposed to be
-- very long. Some optimizations are done though, as names can only consist of a-zA-Z0-9_ (63 characters),
-- we only need to sort on 6 instead of 8 bits per characters.
-- IDs are assigned sequentially starting from 0.
local let link_names [n] (input: []u8) (tokens: [n]tokenref): [n]u32 =
    let (_, offsets, lengths) = unzip3 tokens
    -- a-zA-Z0-9_ are 26 + 26 + 10 + 1= 63 characters, plus one for the out-of-bounds value, so 6 bits will do nicely.
    let bits_per_char = 6
    -- Map characters allowed in a function name to its 6-bit representation.
    -- Note: This function maps a char to 1-63 instead of 0-62, as the out-of-bounds value (0) needs to
    -- compare different than any of the other characters in order to obtain a good sorting.
    let char_to_value (c: u8): u8 =
        if c >= 'a' && c <= 'z' then c - 'a' + 1
        else if c >= 'A' && c <= 'Z' then c - 'A' + 27
        else if c >= '0' && c <= '9' then c - '0' + 53
        else 63 -- '_'
    -- Get a particular bit in the string at `index`.
    let get_name_bit (bit: i32) (index: i32): i32 =
        let bit_in_char = bit % bits_per_char
        let byte_in_string = bit / bits_per_char
        in if byte_in_string >= lengths[index] then 0 else -- Return 0 if out of bounds.
        let c = input[offsets[index] + byte_in_string]
        in u8.get_bit bit_in_char (char_to_value c)
    -- To finally assign an ID to ever string, we need to know whether it is equal to another string.
    -- This function performs a simple linear check.
    let str_eq (a: i32) (b: i32): bool =
        let len_a = i64.i32 lengths[a]
        let len_b = i64.i32 lengths[b]
        in if len_a != len_b then false else
        let off_a = i64.i32 offsets[a]
        let off_b = i64.i32 offsets[b]
        let str_a = (input[off_a : off_a + len_a]) :> [len_a]u8
        let str_b = (input[off_b : off_b + len_a]) :> [len_a]u8
        in map2 (==) str_a str_b |> reduce (&&) true
    -- Compute the amount of bits we need to perform the radix sort on.
    let sort_bits = bits_per_char * i32.maximum lengths
    -- Compute the ordering of the strings by radix sorting. Instead of copying the strings all the time,
    -- simply perform an argsort.
    let order =
        iota n
        |> map i32.i64
        |> radix_sort sort_bits get_name_bit
    -- Compute the (sorted) IDs for each string.
    let vs =
        iota n
        -- First, build a mapping of whether this string is equal to its previous.
        |> map (\i -> if i == 0 then false else str_eq order[i] order[i - 1])
        -- Invert this mapping, so that we get a mask whether this string is the first of a
        -- sequence of equal strings.
        |> map (\x -> !x)
        -- Perform an (exclusive) scan to get the IDs.
        |> map u32.bool
        |> scan (+) 0
        |> map (\x -> x - 1)
    -- Unsort this list of IDs to gain the final ID mapping.
    in scatter
        (replicate n 0u32)
        (map i64.i32 order)
        vs

-- | This pass lexes the input file and produces a list of tokens (which are to be
-- fed into the parser).
let tokenize (input: []u8) (lt: lex_table []) =
    lexer.lex input lt
    -- Filter out tokens whitespace tokens (which should be ignored by the parser).
    |> filter (\(t, _, _) -> t != token_whitespace && t != token_comment && t != token_binary_minus_whitespace)

-- | This function builds a data vector for the token types, containing the following elements:
-- - For each atom_name, a unique 32-bit integer for the name associated to the atom.
-- - For each atom_int_literal, the int's value as 32-bit integer.
-- - For each atom_float_literal, the float's value as 32-bit float (reinterpreted as 32-bit int).
-- As each production is associated with at most one data element,
-- **warning** This function relies on the property that the relative ordering of each atom_int,
-- atom_float and atom_name does not change.
let build_data_vector [n] (node_types: [n]production.t) (input: []u8) (tokens: []tokenref): [n]u32 =
    let has_name ty =
        ty == production_atom_name
        || ty == production_atom_fn_call
        || ty == production_atom_decl
        || ty == production_atom_decl_explicit
        || ty == production_fn_decl
    let pairwise op (a1, b1, c1) (a2, b2, c2) = (op a1 a2, op b1 b2, op c1 c2)
    -- Partition tokens into interesting types.
    let (int_tokens, float_tokens, name_tokens, _) =
        tokens
        |> partition3
            (\(t, _, _) ->
                if t == token_int_literal then 0
                else if t == token_float_literal then 1
                else if t == token_name then 2
                else 3)
    -- Map each token to its semantic value.
    let ints = map (parse_int input) int_tokens
    let floats = map (parse_float input) float_tokens |> map f32.to_bits
    let names = link_names input name_tokens
    -- Now, compute offsets for each type of these tokens in the types array,
    -- similar to how its done in the partition function.
    in
        node_types
        |> map (\ty ->
            if ty == production_atom_int then (1, 0, 0)
            else if ty == production_atom_float then (0, 1, 0)
            else if has_name ty then (0, 0, 1)
            else (0, 0, 0))
        |> scan (pairwise (+)) (0, 0, 0)
        |> map2
            (\ty (int_off, float_off, name_off) ->
                if ty == production_atom_int then ints[int_off - 1]
                else if ty == production_atom_float then floats[float_off - 1]
                else if has_name ty then names[name_off - 1]
                else 0)
            node_types
