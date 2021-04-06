import "util"
import "../../../gen/pareas_grammar"

-- | This pass replaces atom_id which has no application with just atom_id (removes the application)
-- and replaces atom_id with an application with atom_fn.
-- Also replaces no_args with arg_list_end, and replaces args with arg_list.
let fix_fn_args [n] (types: [n]production.t) (parents: [n]i32): ([n]production.t, [n]i32) =
    -- First, for every app, scatter atom_fn.
    let new_types =
        let is =
            types
            |> map2 (\parent ty -> if ty == production_app then parent else -1) parents
            |> map i64.i32
        in
            scatter
                (copy types)
                is
                (replicate n production_atom_fn)
            -- Replace those no_arg nodes with arg_list_end nodes.
            |> map (\ty -> if ty == production_no_args then production_arg_list_end else ty)
            -- Replace those arg nodes with arg_list nodes.
            |> map (\ty -> if ty == production_args then production_arg_list else ty)
    -- Now, simply remove app and no_app.
    let new_parents =
        new_types
        |> map (\ty -> ty == production_app || ty == production_no_app)
        |> remove_nodes parents
    in (new_types, new_parents)

-- | This pass fixes binding expressions. Since they're not quite operator lists (although similar).
-- they need some special handling.
let fix_binds [n] (types: [n]production.t) (parents: [n]i32) =
    -- Scatter up the no_binding nodes.
    let new_types =
        let is =
            types
            |> map2 (\parent ty -> if ty == production_no_binding then parent else -1) parents
            |> map i64.i32
        in
            scatter
                (copy types)
                is
                types
    -- And simply remove binding and no_binding type nodes.
    let new_parents =
        new_types
        |> map (\ty -> ty == production_no_binding || ty == production_binding)
        |> remove_nodes parents
    in (new_types, new_parents)
