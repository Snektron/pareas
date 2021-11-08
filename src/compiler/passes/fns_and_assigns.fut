import "util"
import "../../../gen/pareas_grammar"

-- | This pass replaces `atom_name` depending on it's children:
-- - If it has an application, its translated into `atom_fn_call`.
-- - If it has a declaration, its translated into `atom_decl`.
-- - If it has both, `false` is returned along with the new parents and node types array.
--   Otherwise true
let fix_names [n] (node_types: [n]production.t) (parents: [n]i32): (bool, [n]production.t, [n]i32) =
    -- Scatter up whether the name is a declaration or a function call.
    let is_call =
        let is =
            node_types
            |> map (== production_app)
            |> map2 (\parent is_decl -> if is_decl then parent else -1) parents
            |> map i64.i32
        in scatter
            (replicate n false)
            is
            (replicate n true)
    -- All nodes should either be declarations or calls (or none), but not both.
    -- The grammar needs to allow `maybe_app` in `atom_decl` in order for the parser to be able to handle it,
    -- and we simply check it manually here.
    let invalid =
        node_types
        |> map (== production_atom_decl)
        |> map2 (&&) is_call
        |> reduce (||) false
    -- Compute the new node types according to these masks.
    let node_types =
        map2
            (\nty call -> if call then production_atom_fn_call else nty)
            node_types
            is_call
    -- And finally filter out the now no longer useful node types.
    let parents =
        node_types
        |> map (\nty -> nty == production_app || nty == production_no_app)
        -- There should only be one iteration each.
        |> remove_nodes_lin parents
    in (!invalid, node_types, parents)

-- | This pass removes `no_ascription` and `ascription` nodes, as well as `ascript` which do not have an `ascription`
-- as child. Returns a new parents array.
let fix_ascriptions [n] (node_types: [n]production.t) (parents: [n]i32): [n]i32 =
    -- `ascript` parents of `no_ascription` nodes should be removed also.
    -- First, scatter those up
    let is =
        node_types
        |> map (== production_no_ascription)
        |> map2 (\parent not_ascription -> if not_ascription then parent else -1) parents
        |> map i64.i32
    in scatter
        (replicate n false)
        is
        (replicate n true)
    -- Now also mark `ascription` and `no_ascription` nodes.
    |> map2
        (||)
        (map (\nty -> nty == production_no_ascription || nty == production_ascription) node_types)
    -- These should only be one or two iterations for every node.
    |> remove_nodes_lin parents

-- | The parser cannot parse something else than an expression at a function declaration, so this pass
-- validates that the left child of a `fn_decl` is an `ascript` with a `fn_call` as child.
-- This pass also modifies `fn_decls` in to a more useful node, by removing the `ascript` and the `call` node.
-- This will have as effect that we can treat the `fn_decl` node as having a name associated to it in `tokenizer`,
-- and the new children of a `fn_decl` will be in order an `arg_list`, a `type`, and a `compound_expr`.
-- Returns whether the input is valid and the new parents array.
let fix_fn_decls [n] (node_types: [n]production.t) (parents: [n]i32): (bool, [n]i32) =
    -- We are simply going to check for `atom_fn_call`->`ascript`->`fn_decl` patterns,
    -- and then scatter those values to all `fn_decls` to check whether they are valid.
    let grandparents = map (\parent -> if parent == -1 then -1 else parents[parent]) parents
    -- First, build some masks for the relevant types:
    -- - 'prototypes' are `atom_fn_call` nodes with an `ascript` as parent and a `fn_decl` as grandparent.
    -- - 'fn ascripts' are `ascripts` nodes with a `fn_decl` as parent.
    let fn_protos =
        map3
            (\nty parent grandparent ->
                nty == production_atom_fn_call
                -- Accessing the parent is fine here as `atom_fn_call` should never be the root node.
                && node_types[parent] == production_ascript
                -- Accessing the parent is fine here as `ascript` should never be the root node.
                && node_types[grandparent] == production_fn_decl)
        node_types
        parents
        grandparents
    let fn_ascripts =
        map2
            (\nty parent ->
                nty == production_ascript
                -- Accessing the parent is fine here as `ascript` should never be the root node.
                && node_types[parent] == production_fn_decl)
            node_types
            parents
    let valid =
        -- Now, compute which decls are valid by scattering fn_protos to the grandparent.
        let is =
            fn_protos
            |> map2 (\grandparent is_proto -> if is_proto then grandparent else -1) grandparents
            |> map i64.i32
        in scatter
            (replicate n false)
            is
            (replicate n true)
        -- These values must hold for all `fn_decl`s.
        |> map2
            (==)
            (map (== production_fn_decl) node_types)
        |> reduce (&&) true
    -- Finally, compute the new parents simply by removing the scripts and protos
    let parents =
        map2 (||) fn_protos fn_ascripts
        -- This should only be max 2 iterations each so use the linear version.
        |> remove_nodes_lin parents
    in (valid, parents)

-- | A small pass that replaces `no_args` and `args` with an `arg_list`.
-- Should happen somewhere before `fix_param_lists`.
let reinsert_arg_lists [n] (node_types: [n]production.t): [n]production.t =
    map
        (\nty ->
            if nty == production_no_args || nty == production_args then production_arg_list
            else nty)
        node_types

-- | It is useful for codegen and futher down the frontend part to tell an arg node from a param node, so this pass
-- simply inserts those.
let fix_param_lists [n] (node_types: [n]production.t) (parents: [n]i32): [n]production.t =
    let grandparents = map (\parent -> if parent == -1 then -1 else parents[parent]) parents
    let node_types =
        map3
            (\nty parent grandparent ->
                -- We can access grandparent and parent node types here fine because `arg` and `arg_list` will never
                -- be root nodes.
                if nty == production_arg && node_types[grandparent] == production_fn_decl then production_param
                else if nty == production_arg_list && node_types[parent] == production_fn_decl then production_param_list
                else nty)
            node_types
            parents
            grandparents
    in node_types

-- | Function parameters should be an `atom_name` with an `ascript`. This check is performed here.
-- Just like in `fix_fn_decls`, we're going to look for a pattern and then scatter up.
-- TODO: Maybe those two stages can be merged?
let check_fn_params [n] (node_types: [n]production.t) (parents: [n]i32): bool =
    let grandparents = map (\parent -> if parent == -1 then -1 else parents[parent]) parents
    let is =
        -- First, build a vector of parameter `atom_name`s
        map3
            (\nty parent grandparent ->
                nty == production_atom_name
                -- Accessing these parents should be fine.
                && node_types[parent] == production_ascript
                && node_types[grandparent] == production_param)
            node_types
            parents
            grandparents
        |> map2 (\grandparent is_param_name -> if is_param_name then grandparent else -1) grandparents
        |> map i64.i32
    -- Scatter to grandparent, the `param` node.
    in scatter
        (replicate n false)
        is
        (replicate n true)
    |> map2
        (==)
        (map (== production_param) node_types)
    -- Must hold for every node
    |> reduce (&&) true

-- | In this pass `atom_decl` nodes are combined with `ascript` nodes to form `atom_decl_explicit` nodes.
-- | In this pass, `atom_decl_explicit` nodes are inserted. This is done in two occasions:
-- - An `atom_decl` which is child of an `ascript`.
-- - An `atom_name` which is child of an `ascript` which in turn is child of `param`.
let squish_decl_ascripts [n] (node_types: [n]production.t) (parents: [n]i32): ([n]production.t, [n]i32) =
    let grandparents = map (\parent -> if parent == -1 then -1 else parents[parent]) parents
    -- We're simply going to check for these patterns from the children, and scatter their properties up to the
    -- parents, and then removing the old nodes.
    let to_replace =
        map3
            (\nty parent grandparent ->
                -- Accessing the parent is fine for both of these, `atom_decl` and `ascript` will never be root nodes.
                (nty == production_atom_decl
                    && node_types[parent] == production_ascript)
                || (nty == production_atom_name
                    && node_types[parent] == production_ascript
                    && node_types[grandparent] == production_param))
            node_types
            parents
            grandparents
    let is =
        to_replace
        |> map2 (\parent replace -> if replace then parent else -1) parents
        |> map i64.i32
    let is_explicit_decl = scatter (replicate n false) is (replicate n true)
    -- Replace the ascripts with explicit declarations.
    let node_types =
        map2
            (\nty explicit_decl -> if explicit_decl then production_atom_decl_explicit else nty)
            node_types
            is_explicit_decl
    -- And finally, remove those old nodes we no longer need.
    -- This should only be 1 iteration at most.
    let parents = remove_nodes_lin parents to_replace
    in (node_types, parents)


-- | This function checks whether the left child of all `assign` nodes is either an `atom_decl`, `atom_decl_explicit`
-- or an `atom_name`.
-- Note: Performing this function after `remove_marker_nodes` makes `(a: int) = x` valid, but it saves a
-- reduce_by_index and so is probably justified.
-- TODO: Maybe check with `insert_derefs`?
-- TODO: Also check whether declarations are _only_ LHS of assigns, and not free standing.
let check_assignments [n] (node_types: [n]production.t) (parents: [n]i32) (prev_sibling: [n]i32): bool =
    prev_sibling
    -- First, build a mask of whether this node is the first child of its parent.
    |> map (== -1)
    -- Build a new mask which states whether a node is the left child of an assignment.
    |> map2
        (\parent first_child -> first_child && parent != -1 && node_types[parent] == production_assign)
        parents
    -- If the parent is an assignment and the node is the left child of its parent, it should be an l-value producing node,
    -- either variable name or a variable declaration.
    |> map2
        (\nty parent_is_assignment -> !parent_is_assignment || node_produces_reference nty)
        node_types
    -- Above should hold for all nodes.
    |> reduce (&&) true

-- | The backend needs to explicitly know when an l-value needs to be transformed into an r-value. Unfortunately,
-- this requires us to insert nodes, which happens in this pass.
-- Note: In order to prevent a second radix sort, we patch the prev_sibling vector. Sadly, this is harder for the
-- depths vector, so we'll need to recompute that.
-- TODO: This can also be implemented by inserting dummy nodes in the grammar and removing those. That would
-- require a reduce_by_index to check if a node is the first child (as it would need to happen before sibling
-- array computation), but saves the depth-recomputation and the nasty code to add new nodes.
let insert_derefs [n] (node_types: [n]production.t) (parents: [n]i32) (prev_sibling: [n]i32): [](production.t, i32, i32) =
    -- Technically we need to know the child's index, but all relevant nodes here are binary and thus
    -- simply knowing the first child from the others is sufficient.
    let is_first_child = map (== -1) prev_sibling
    -- Create a mask whether the result of this node is an lvalue-expression.
    let result_is_lvalue = map node_produces_reference node_types
    -- Build a mask of which nodes need a new dereference parent inserted.
    -- A dereference needs to occur if this node produces an l-value, and it's parent expects an r-value.
    -- This always happens, except when the node is the left child of an assignment node, or when the parent
    -- is a statement expression (in which case the result is discarded), or when the node is in a param list.
    let needs_deref =
        map2
            (\first_child parent ->
                parent == -1
                || (node_types[parent] == production_assign && first_child)
                || node_types[parent] == production_stat_expr
                || node_types[parent] == production_param)
            is_first_child
            parents
        |> map (\x -> !x)
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
    let additional_node_types = replicate m production_atom_unary_deref
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
        ((node_types ++ additional_node_types) :> [k]production.t)
        ((parents ++ additional_parents) :> [k]i32)
        ((prev_sibling ++ additional_prev_siblings) :> [k]i32)
