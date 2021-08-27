module lexer = import "../compiler/lexer/lexer"
import "../compiler/parser/parser"
import "../compiler/util"

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

-- Code taken from pareas itself
-- See compiler/passes/util.fut and compiler/passes/compactify.fut for more info

let find_unmarked_parents_log [n] (parents: [n]i32) (marks: [n]bool): [n]i32 =
    iterate
        (n |> i32.i64 |> bit_width)
        (\links ->
            map
                (\link -> if link == -1 || !marks[link] then link else links[link])
                links)
        parents

let remove_nodes_log [n] (parents: [n]i32) (remove: [n]bool): [n]i32 =
    find_unmarked_parents_log parents remove
    |> map3
        (\i remove parent -> if remove then i else parent)
        (iota n |> map i32.i64)
        remove

let compactify [n] (parents: [n]i32): [](i32, i32) =
    -- TODO: Mark all nodes of deleted subtrees as deleted by setting their parents to themselves.
    -- Make a mask specifying whether a node should be included in the new tree.
    let include_mask =
        iota n
        |> map i32.i64
        |> zip parents
        |> map (\(i, parent) -> parent != i)
    let is =
        include_mask
        |> map i32.bool
        |> scan (+) 0
    -- break up the computation of is temporarily to get the size of the new arrays.
    let m = last is |> i64.i32
    -- For a node index i in the old array, this array gives the position in the new array (which should be of size m)
    let new_index =
        map2 (\inc i -> if inc then i else -1) include_mask is
        |> map (+ -1)
    -- For a node index j in the new array, this gives the position in the old array
    let old_index =
        scatter
            (replicate m 0i32)
            (new_index |> map i64.i32)
            (iota n |> map i32.i64)
    -- Also compute the new parents array here, since we need the `is` array for it, but dont need it anywhere else.
    let parents =
        -- Begin with the indices into the old array
        old_index
        -- Gather its parent, which points to an index into the old array as well
        |> gather parents
        -- Find the index into the new array
        |> map (\i -> if i == -1 then -1 else new_index[i])
    in zip parents old_index

-- Json entry points

entry json_lex (input: []u8) (lt: lex_table []): []token.t =
    lexer.lex input lt
    |> map (.0)
    |> filter (!= token_whitespace)

entry json_parse (tokens: []token.t) (sct: stack_change_table []) (pt: parse_table []): (bool, []production.t) =
    if json_parser.check tokens sct
        then (true, json_parser.parse tokens pt)
        else (false, [])

entry json_build_parse_tree [n] (node_types: [n]production.t) (arities: arity_array): []i32 =
    json_parser.build_parent_vector node_types arities

-- | Restructure the json tree:
-- - Non relevant nodes are removed.
-- - Lists are flattened.
-- - String->member pairs are squashed.
-- - Tree is compactified.
entry json_restructure [n] (node_types: [n]production.t) (parents: [n]i32): ([]production.t, []i32) =
    let parents =
        node_types
        |> map (\nty -> nty == production_values
            || nty == production_value_list
            || nty == production_value_list_end
            || nty == production_no_member
            || nty == production_no_values)
        |> remove_nodes_log parents
    -- Squash string->member to member.
    -- Note: member can only have a string as parent. For hypothetical lexeme extraction, the member node would
    -- be matched with the string's lexeme.
    let parents =
        let is_member = map (== production_member) node_types
        let is =
            map2 (\parent is_member -> if is_member then parent else -1) parents is_member
            |> map i64.i32
        let strings_to_remove = scatter
            (replicate n false)
            is
            (replicate n true)
        let get_new_parent self parent is_string_to_remove is_member =
            if is_string_to_remove then self
            else if is_member then parents[parent]
            else parent
        in
            map4
                get_new_parent
                (iota n |> map i32.i64)
                parents
                strings_to_remove
                is_member
    -- *really* remove the old nodes
    let (parents, old_index) = compactify parents |> unzip
    let node_types = gather node_types old_index
    in (node_types, parents)

-- | Validate that the children of objects are members, and the parents of members are objects.
entry json_validate [n] (node_types: [n]production.t) (parents: [n]i32): bool =
    map2
        (\nty parent -> (nty == production_member) == (parent != -1 && node_types[parent] == production_object))
        node_types
        parents
    |> reduce (&&) true

