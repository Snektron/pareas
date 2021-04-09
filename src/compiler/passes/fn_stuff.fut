import "util"
import "../../../gen/pareas_grammar"

-- | This pass replaces atom_id which has no application with just atom_id (removes the application)
-- and replaces atom_id with an application with atom_fn_call.
-- Also replaces no_args with arg_list_end, and replaces args with arg_list.
let fix_fn_args [n] (types: [n]production.t) (parents: [n]i32): ([n]production.t, [n]i32) =
    -- First, for every app, scatter atom_fn.
    let new_types =
        let is =
            types
            |> map (== production_app)
            |> map2 (\parent is_app -> if is_app then parent else -1) parents
            |> map i64.i32
        in
            scatter
                (copy types)
                is
                (replicate n production_atom_fn_call)
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

-- | This pass eliminates `bind` and `no_bind` type nodes by squishing them with their parents:
-- - If the parent of a bind is an atom_fn, it changes into an atom_fn_proto.
-- - If the parent of a bind is an atom_id, it changes into an atom_decl.
-- - `bind` and `no_bind` type nodes are simply removed after.
-- This pass should be performed after `fix_fn_args`@term.
let squish_binds [n] (types: [n]production.t) (parents: [n]i32) =
    -- First, move up the 'bind-ness' up to the parents
    let is_bind_parent =
        let is =
            types
            |> map (== production_bind)
            |> map2 (\parent is_bind -> if is_bind then parent else -1) parents
            |> map i64.i32
        in scatter
            (replicate n false)
            is
            (replicate n true)
    -- Perform the relevant transformations.
    let new_types =
        map2
            -- Note that bind_parent should not be true for types other than production_atom_fn_call and
            -- production_atom_id, which should be guaranteed by the grammar and the result of `fix_fn_args`@term.
            (\bind_parent ty ->
                if bind_parent && ty == production_atom_fn_call then production_atom_fn_proto
                else if bind_parent && ty == production_atom_id then production_atom_decl
                else ty)
            is_bind_parent
            types
    -- Finally, remove old `bind` and `no_bind` type nodes.
    let new_parents =
        new_types
        |> map (\ty -> ty == production_bind || ty == production_no_bind)
        |> remove_nodes parents
    in (new_types, new_parents)
