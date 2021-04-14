import "util"
import "../../../gen/pareas_grammar"

let resolve_fns [n] (types: [n]production.t) (parents: [n]i32) (data: [n]u32) =
    -- The program is only valid if all functions are unique, and so we must check whether all
    -- elements in the data array corresponding with atom_fn_proto are unique. There are multiple
    -- ways to do this:
    -- - Use the id to extract all function name identifiers (the value in data) and then perform
    --   a radix sort and checking adjacent values for uniqueness.
    -- - Use reduce_by_index over the function name identifier to count the amounts of identifiers
    --   and check if they are all <= 1. This requires more auxillary data, but should be a little
    --   faster when there are many functions that have a different name (which is assumed to be the
    --   general case).
    -- TODO: Maybe merge fn_id_by_name and the computation of all_unique?
    -- TODO: Maybe merge `fn_decl` and `atom_fn_proto` in some stage before this?
    let is_fn_proto = map (== production_atom_fn_proto) types
    let is_fn_call = map (== production_atom_fn_call) types
    let fn_id =
        is_fn_proto
        |> map i32.bool
        |> scan (+) 0
        |> map (+ -1)
    let is =
        data
        |> map i64.u32
        |> map2 (\is_fn_proto name_id -> if is_fn_proto then name_id else -1) is_fn_proto
    let all_unique =
        reduce_by_index
            -- n is quite a large upper bound here (we know the maximum function name identifier
            -- cannot be larger than the amount of nodes, as each unique name is given an id sequentially
            -- from 0 (see `ident_link`@term@"tokenize").
            -- We could replace this with the amount of unique identifiers in the program (which would
            -- need to be passed from `build_data_vector`@term@"tokenize"), or we could simply compute the
            -- largest ID here.
            -- Note: At this point, the tree is smaller than the original source, however, no nodes carrying
            -- semantic information (such as ids) have been removed, so this is still a valid upper bound.
            (replicate n 0i32)
            (+)
            0
            is
            (replicate n 1i32)
        |> all (<= 1)
    let fn_id_by_name =
        scatter
            (replicate n (-1i32))
            is
            fn_id
    let calls_valid =
        data
        |> map i32.u32
        |> map2 (\is_call name_id -> if is_call then fn_id_by_name[name_id] != -1 else true) is_fn_call
        |> reduce (&&) true
    -- Compute the new data vector by, for every `fn_call` type node, looking up its name id in the fn_id_by_name
    -- array.
    let new_data =
        map2
            (\is_call data -> if is_call then u32.i32 fn_id_by_name[i32.u32 data] else data)
            is_fn_call
            data
    -- Also scatter fn_id's to their appropriate positions
    let new_data =
        let is =
            is_fn_proto
            |> map2 (\parent is_proto -> if is_proto then parent else -1) parents
            |> map i64.i32
        in
            scatter
                new_data
                is
                (fn_id |> map u32.i32)
    in (all_unique, new_data)
