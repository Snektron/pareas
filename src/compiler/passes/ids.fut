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
let assign_ids [n] (node_types: [n]production.t) (resolution: [n]i32) (data_types: [n]data_type) (data: [n]u32) =
    -- Even though the tree is strictly speaking not in any order right now, there is no pass that
    -- changes the relative order of the nodes we're interested in (`fn_decl`, `atom_decl` and `param`),
    -- so we're just going to do a (segmented) scan to assign the IDs. Function IDs will simply be a
    -- normal exclusive scan, and the function declarations also acts as flag to reset the counters for
    -- the `param` and `decl` counters.
    let is_fn_decl = map (== production_fn_decl) node_types
    let fn_ids =
        is_fn_decl
        |> map u32.bool
        |> scan (+) 0
        -- `scan` is inclusive, so simply subtract one to make it exclusive. This does overflow
        -- for some nodes though, but they should be ignored anyway.
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
                    if dty == data_type.int then (0, 1, 0) else (0, 0, 1)
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
    in
        map2
            (\res d -> if res != -1 then data[res] else d)
            resolution
            data