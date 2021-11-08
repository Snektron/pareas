import "util"
import "../datatypes"
import "../../../gen/pareas_grammar"
import "../../../lib/github.com/diku-dk/segmented/segmented"

-- | In this pass, we're going to assign a few IDs and offsets, which are all sequential and start at
-- zero, in the data vector:
-- - `fn_decl` nodes gain a function ID.
-- - `atom_decl` and `atom_decl_explicit` gain a function-based offset ID.
-- - `param` nodes gain an offset ID based on their type.
-- - Finally, `arg`, `fn_call` and `atom_name` get IDs based on the node that the resolution vector
--   points to.
-- This function also computes a function table, which simply contains the amount of declarations indexed
-- by function-ID. The required arrays are computed in this function anyway.
let assign_ids [n] (node_types: [n]production.t) (resolution: [n]i32) (data_types: [n]data_type) (data: [n]u32): ([n]u32, []u32) =
    -- Even though the tree is strictly speaking not in any order right now, there is no pass that
    -- changes the relative order of the nodes we're interested in (`fn_decl`, `atom_decl` and `param`),
    -- so we're just going to do a (segmented) scan to assign the IDs. Function IDs will simply be a
    -- normal exclusive scan, and the function declarations also acts as flag to reset the counters for
    -- the `param` and `decl` counters.
    let is_fn_decl = map (== production_fn_decl) node_types
    let num_fn_decls = is_fn_decl |> map u32.bool |> reduce (+) 0
    let fn_ids =
        is_fn_decl
        |> map u32.bool
        |> scan (+) 0
        -- `scan` is inclusive, so simply subtract one to make it exclusive. This does overflow
        -- for some nodes though (before the first decl), but they should be ignored anyway.
        |> map (\a -> a - 1)
    -- Compute the declaration, int parameter and float parameter IDs simultaneously.
    let (decl_ids, int_param_ids, float_param_ids) =
        -- Map to different counters for the different parameter types.
        -- Note that at this point, parameter types should only be reference types, so either int ref
        -- or float ref.
        map2
            (\nty dty ->
                if nty == production_atom_decl || nty == production_atom_decl_explicit then
                    (1, 0, 0)
                else if nty == production_param then
                    if dty == data_type.int_ref then (0, 1, 0) else (0, 0, 1)
                else
                    (0, 0, 0))
            node_types
            data_types
        -- Perform an exclusive segmented scan over these, where `fn_decl` resets.
        |> segmented_scan
            (\(a0, a1, a2) (b0, b1, b2) -> (a0 + b0, a1 + b1, a2 + b2))
            (0u32, 0u32, 0u32)
            is_fn_decl
        -- Again, make the scan exclusive simply by subtracting one.
        |> map (\(a, b, c) -> (a - 1, b - 1, c - 1))
        |> unzip3
    -- Compute the maximum decl per function by scattering.
    -- At this point, `fn_id` contains for every node the function ID which that node is defined in (except for the
    -- `fn_decl_list` at the root). This means we can compute the maximum decl simply by checking whether this decl is the
    -- maximum (when the next node is a fn_decl or the node is the last node) and then scatter that to the appropriate place
    -- in the function table.
    let fn_tab =
        let is =
            iota n
            |> map (\i -> i == n - 1 || is_fn_decl[i + 1])
            |> map2 (\fn_id is_last_decl -> if is_last_decl then i64.u32 fn_id else -1) fn_ids
        in
            scatter
                (replicate (i64.u32 num_fn_decls) 0u32)
                is
                -- Add one to get a maximum declaration instead of a count.
                (map (+1) decl_ids)
    -- Insert everything back into the data vector.
    let data =
        map4
            (\nty dty d (fn_id, int_param_id, float_param_id, decl_id) ->
                if nty == production_fn_decl then
                    fn_id
                else if nty == production_param then
                    if dty == data_type.int_ref then int_param_id else float_param_id
                else if nty == production_atom_decl || nty == production_atom_decl_explicit then
                    decl_id
                else
                    d)
        node_types
        data_types
        data
        (zip4 fn_ids int_param_ids float_param_ids decl_ids)
    -- Finally, simply fetch the value for every node where the resolution is not -1.
    let data =
        map2
            (\res d -> if res != -1 then data[res] else d)
            resolution
            data
    in (data, fn_tab)
