import "util"
import "../../../gen/pareas_grammar"

let resolve_fns [n] (types: [n]production.t) (parents: [n]i32) (data: [n]u32) =
    -- TODO: Maybe merge `fn_decl` and `atom_fn_proto` in some stage before this?
    -- Extract all prototypes (which are children of fn_decls).
    let is_fn_proto = map (== production_atom_fn_proto) types
    let is_fn_call = map (== production_atom_fn_call) types
    -- The program is only valid if all functions are unique, and so we must check whether all
    -- elements in the data array corresponding with atom_fn_proto are unique. There are multiple
    -- ways to do this:
    -- - Use the id to extract all function name identifiers (the value in data) and then perform
    --   a radix sort and checking adjacent values for uniqueness. This requires an expensive lookup
    --   operation during resolving calls.
    -- - Use reduce_by_index over the function name identifier to count the amounts of identifiers
    --   and check if they are all <= 1. This requires more auxillary data, but should be a little
    --   faster when there are many functions that have a different name (which is assumed to be the
    --   general case). This has also the advantage that resolving function calls is more efficient,
    --   as we can re-use this array to store the function id.
    -- TODO: Experiment some with these methods.
    let not_function = -1i32
    let duplicate = -2i32
    -- Fused computation of fn_decl node by name and checking of duplicate declarations.
    let fn_decl_by_name =
        let is =
            data
            |> map i64.u32
            |> map2 (\is_fn_proto name_id -> if is_fn_proto then name_id else -1) is_fn_proto
        in
            reduce_by_index
                -- n is quite a large upper bound here (we know the maximum function name identifier
                -- cannot be larger than the amount of nodes, as each unique name is given an id sequentially
                -- from 0 (see `ident_link`@term@"tokenize").
                -- We could replace this with the amount of unique identifiers in the program (which would
                -- need to be passed from `build_data_vector`@term@"tokenize"), or we could simply compute the
                -- largest ID here.
                (replicate n not_function)
                (\a b ->
                    if a == duplicate || b == duplicate then duplicate
                    else if a == not_function then b
                    else if b == not_function then a
                    else if a == b then duplicate
                    else not_function)
                not_function
                is
                -- We want to get the index of the `fn_decl`, which is the parent of a fn_proto.
                -- We already make sure that only values corresponding to fn_proto get scattered in the
                -- computation of `is`, so we can just scatter the parents here.
                parents
    let all_unique = all (!= duplicate) fn_decl_by_name
    -- Check if all calls are valid by looking up in the fn_decl_by_name array and checking if it's not not_function.
    let calls_valid =
        data
        |> map i32.u32
        |> map2 (\is_call name_id -> if is_call then fn_decl_by_name[name_id] != not_function else true) is_fn_call
        |> reduce (&&) true
    -- Compute the new data vector by, for every `fn_call` type node, looking up its name id in the fn_decl_by_name
    -- array.
    let new_data =
        map2
            (\is_call data -> if is_call then u32.i32 fn_decl_by_name[i32.u32 data] else data)
            is_fn_call
            data
    in (all_unique && calls_valid, new_data)
