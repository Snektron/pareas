import "util"
import "../../../gen/pareas_grammar"

-- | This pass replaces atom_name which has no application with just atom_name (removes the application)
-- and replaces atom_name with an application with atom_fn_call.
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
        -- These should only be one iteration each.
        |> remove_nodes_lin parents
    in (new_types, new_parents)

-- | This pass eliminates `bind` and `no_bind` type nodes by squishing them with their parents:
-- - If the parent of a bind is an atom_fn, it changes into an atom_fn_proto.
-- - If the parent of a bind is an atom_name, it changes into an atom_decl.
-- - `bind` and `no_bind` type nodes are simply removed after.
-- This pass should be performed after `fix_fn_args`@term.
let squish_binds [n] (types: [n]production.t) (parents: [n]i32): ([n]production.t, [n]i32) =
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
            -- production_atom_name, which should be guaranteed by the grammar and the result of `fix_fn_args`@term.
            (\bind_parent ty ->
                if bind_parent && ty == production_atom_fn_call then production_atom_fn_proto
                else if bind_parent && ty == production_atom_name then production_atom_decl
                else ty)
            is_bind_parent
            types
    -- Finally, remove old `bind` and `no_bind` type nodes.
    let new_parents =
        new_types
        |> map (\ty -> ty == production_bind || ty == production_no_bind)
        -- These should only be one iteration each.
        |> remove_nodes_lin parents
    in (new_types, new_parents)

-- | This pass checks whether the structure of function declaration argument lists are correct, and is supposed to
-- be performed somewhere after `squish_binds`@term, but before `remove_marker_nodes`@term@"remove_marker_nodes".
-- This pass should only be performed _after_` `flatten_lists`@term@"flatten_lists".
let check_fn_params [n] (types: [n]production.t) (parents: [n]i32): bool =
    parents
    -- Check if the grandparent is a `fn_proto`.
    -- Parents of a production_arg_list should never be -1, so checking directly is fine.
    |> map (\parent -> parent != -1 && types[parent] == production_arg_list && types[parents[parent]] == production_atom_fn_proto)
    -- If the grandparent is a `fn_proto` (and the parent is a production_arg_list), then this node should be a declaration.
    |> map2
        (\ty is_param -> if is_param then ty == production_atom_decl else true)
        types
    -- Should all yield true.
    |> reduce (&&) true

-- | This function checks whether the left child of all `fn_decl` nodes is an `atom_fn_proto` node,
-- and also checks whether the parent of `atom_fn_proto` nodes is a `fn_decl`.
let check_fn_decls [n] (types: [n]production.t) (parents: [n]i32) (prev_sibling: [n]i32): bool =
    prev_sibling
    -- First, build a mask of whether this node is the first child of its parent.
    |> map (== -1)
    -- If so, check whether the node's parent is a function declaration node.
    |> map2
        (\parent first_child -> first_child && parent != -1 && types[parent] == production_fn_decl)
        parents
    -- If a node's parent is a function declaration and the node is the left child, it should be a function
    -- prototype.
    |> map2
        (==)
        (map (== production_atom_fn_proto) types)
    -- Above should hold for all nodes.
    |> reduce (&&) true

-- | This function checks whether the left child of all `assign` nodes is either an `atom_decl` or an `atom_name`.
-- Note: Performing this function after `remove_marker_nodes` makes `(a: int) = x` valid, but it saves a
-- reduce_by_index and so is probably justified.
let check_assignments [n] (types: [n]production.t) (parents: [n]i32) (prev_sibling: [n]i32): bool =
    prev_sibling
    -- First, build a mask of whether this node is the first child of its parent.
    |> map (== -1)
    -- Build a new mask which states whether a node is the left child of an assignment.
    |> map2
        (\parent first_child -> first_child && parent != -1 && types[parent] == production_assign)
        parents
    -- If the parent is an assignment and the node is the left child of its parent, it should be an l-value producing node,
    -- either variable name or a variable declaration.
    |> map2
        (\ty parent_is_assignment -> if parent_is_assignment then ty == production_atom_decl || ty == production_atom_name else true)
        types
    -- Above should hold for all nodes.
    |> reduce (&&) true

-- | The backend needs to explicitly know when an l-value needs to be transformed into an r-value. Unfortunately,
-- this requires us to insert nodes, which happens in this pass.
-- Note: In order to prevent a second radix sort, we patch the prev_sibling vector. Sadly, this is harder for the
-- depths vector, so we'll need to recompute that.
-- TODO: This can also be implemented by inserting dummy nodes in the grammar and removing those. That would
-- require a reduce_by_index to check if a node is the first child (as it would need to happen before sibling
-- array computation), but saves the depth-recomputation and the nasty code to add new nodes.
let insert_derefs [n] (types: [n]production.t) (parents: [n]i32) (prev_sibling: [n]i32): [](production.t, i32, i32) =
    -- Technically we need to know the child's index, but all relevant nodes here are binary and thus
    -- simply knowing the first child from the others is sufficient.
    let is_first_child = map (== -1) prev_sibling
    -- Create a mask whether the result of this node is an lvalue-expression.
    let result_is_lvalue =
        types
        -- If functions returning lvalues was accepted, this would be the place to insert it.
        |> map (\ty -> ty == production_atom_name || ty == production_atom_decl)
    -- Build a mask of which nodes need a new dereference parent inserted.
    -- A dereference needs to occur if this node produces an l-value, and it's parent expects an r-value.
    -- This always happens, except when the node is the left child of an assignment node, or when the parent
    -- is a statement expression (in which case the result is discarded).
    let needs_deref =
        map2
            (\first_child parent ->
                parent != -1
                && !(types[parent] == production_assign && first_child)
                && !(types[parent] == production_stat_expr)
                -- TODO: Merge with check_fn_params?
                && !(types[parent] == production_arg_list && types[parents[parent]] == production_atom_fn_proto))
            is_first_child
            parents
        |> map2 (&&) result_is_lvalue
    -- Count the number of dereference nodes to insert
    let m =
        needs_deref
        |> map i32.bool
        |> reduce (+) 0
        |> i64.i32
    -- Compute the index in the new new nodes array
    let additional_nodes_index =
        needs_deref
        |> map i32.bool
        |> scan (+) 0
        |> map (+ -1)
        |> map2 (\needs_deref i -> if needs_deref then i else -1) needs_deref
    -- Extract additional parents.
    -- We're going to replace the original parents so that prev_sibling pointers remain intact,
    -- and extract the original parents into a new array which will be concatenated to the old.
    let additional_parents =
        scatter
            (replicate m (-1i32))
            (map i64.i32 additional_nodes_index)
            parents
    let additional_types = replicate m production_atom_unary_deref
    -- First, update the sibling pointers to the new indices.
    let prev_sibling =
        map
            (\sibling ->
                if sibling != -1i32 && needs_deref[sibling] then (additional_nodes_index[sibling] + i32.i64 n)
                else sibling)
            prev_sibling
    -- Steal the additional prev sibling from the updated list.
    let additional_prev_siblings =
        scatter
            (replicate m (-1i32))
            (map i64.i32 additional_nodes_index)
            prev_sibling
    -- And update the old nodes' prev sibling pointer. These are all the only child of their new parents.
    let prev_sibling =
        map2
            (\needs_deref prev_sibling -> if needs_deref then -1 else prev_sibling)
            needs_deref
            prev_sibling
    -- Compute new parents array
    let parents =
        additional_nodes_index
        |> map (\i -> if i == -1 then i else i + i32.i64 n)
        |> map2 (\parent i -> if i == -1 then parent else i) parents
    let k = m + n
    -- Zip the return value so futhark can prove theyre all the same size.
    in zip3
        ((types ++ additional_types) :> [k]production.t)
        ((parents ++ additional_parents) :> [k]i32)
        ((prev_sibling ++ additional_prev_siblings) :> [k]i32)
