import "util"
import "../util"
import "../datatypes"
import "../../../gen/pareas_grammar"

-- | A mask for all nodes that should gain a result type, which in general are all expression nodes,
-- along with `fn_decl`, `stat_expr` and `stat_return`.
-- Note that list end and head nodes are already filtered out, as well as a few other nodes.
local let is_expr_node = mk_production_mask [
        -- Technically not an expression node, but is still useful if they gain a type.
        production_fn_decl,
        production_stat_expr,
        production_stat_return,

        production_no_expr,
        production_assign,
        production_ascript,
        production_logical_or,
        production_logical_and,
        production_rela_eq,
        production_rela_neq,
        production_rela_gt,
        production_rela_gte,
        production_rela_lt,
        production_rela_lte,
        production_bitwise_and,
        production_bitwise_or,
        production_bitwise_xor,
        production_shift_lr,
        production_shift_ar,
        production_shift_ll,
        production_sum_add,
        production_sum_sub,
        production_prod_mul,
        production_prod_div,
        production_prod_mod,
        production_atom_unary_neg,
        production_atom_unary_bitflip,
        production_atom_unary_not,
        production_atom_cast,
        -- production_atom_paren, -- Already filtered out.
        production_atom_name,
        production_atom_int,
        production_atom_float,
        production_atom_fn_call,
        production_atom_decl,
        production_atom_decl_explicit,
        production_atom_unary_deref,
        production_arg,
        production_param
    ]

-- | Some nodes have a result type that is not determined by their children, which are
-- encoded in this array.
local let predetermined_result_types = mk_production_array data_type.invalid [
        -- Literals are the obvious case.
        (production_atom_float, data_type.float),
        (production_atom_int, data_type.int),
        -- Some operators are only valid for integers.
        (production_logical_or, data_type.int),
        (production_logical_and, data_type.int),
        (production_bitwise_and, data_type.int),
        (production_bitwise_or, data_type.int),
        (production_bitwise_xor, data_type.int),
        (production_shift_lr, data_type.int),
        (production_shift_ar, data_type.int),
        (production_shift_ll, data_type.int),
        (production_atom_unary_not, data_type.int),
        (production_atom_unary_bitflip, data_type.int),
        -- Relational operators accept floats as children, but their results are always integers.
        (production_rela_eq, data_type.int),
        (production_rela_neq, data_type.int),
        (production_rela_gt, data_type.int),
        (production_rela_gte, data_type.int),
        (production_rela_lt, data_type.int),
        (production_rela_lte, data_type.int),
        -- For void return statements.
        (production_no_expr, data_type.void)
    ]

local let is_relational_op = mk_production_mask [
        production_rela_eq,
        production_rela_neq,
        production_rela_gt,
        production_rela_gte,
        production_rela_lt,
        production_rela_lte
    ]

-- | Translate a node type (such as type_int) into a data type. If its not a `type`-type node,
-- the invalid data type is returned.
local let node_type_to_data_type (nt: production.t): data_type =
    if nt == production_type_void then data_type.void
    else if nt == production_type_int then data_type.int
    else if nt == production_type_float then data_type.float
    else data_type.invalid

-- | This function resolves inherits by following linked-list pointer chains. This function is
-- mostly the same as `find_roots`, except that it also checks whether a pointer passed a
-- dereference-type node (given by `is_deref`).
local let resolve_inherits [n] (parents: [n]i32) (is_deref: [n]bool): ([n]i32, [n]bool) =
    iterate
        (n |> i32.i64 |> bit_width)
        (\(links, loses_reference) ->
            let loses_reference' =
                links
                |> map (\link -> link != -1 && loses_reference[link])
                |> map2 (||) loses_reference
            let links' =
                map
                    (\link -> if link == -1 || links[link] == -1 then link else links[link])
                    links
            in (links', loses_reference'))
        (parents, is_deref)

-- | This pass resolves (but not checks!) a type for each expression-type node.
let resolve_types [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (resolution: [n]i32): [n]data_type =
    -- Initialize the type resolution vector with nodes which inherit their results from their 'type' children.
    -- These include (function) declarations and cast nodes.
    let data_types =
        -- Compute the data type according to the type and it's parent (which might add a reference).
        let vs =
            node_types
            -- Map these data types to their initial types
            |> map node_type_to_data_type
            -- Add a reference for `atom_decl_explicit` and `atom_name` nodes.
            -- Note that `atom_decl` will simpy yield invalid.
            |> map2 (\parent dty ->
                -- Adding a reference to an invalid type simply produces an invalid type, so this is fine.
                if parent != -1 && node_produces_reference node_types[parent] then data_type.add_ref dty
                else dty)
            parents
        -- Compute indices to scatter to: Parents of nodes which have a valid data type in the `vs` vector.
        let is =
            vs
            |> map (!= data_type.invalid)
            |> map2 (\parent valid -> if valid then parent else -1) parents
            |> map i64.i32
        -- And finally scatter these to the parents
        in scatter (replicate n data_type.invalid) is vs
    -- Now initialize the type resolution vector with data types obtained via the 'resolution' vector.
    -- Not this produces an `invalid` value for `arg` types, as they point to the `param` and not the decl.
    let data_types =
        map2
            -- This should be fine to do in-place as they are disjoint, but futhark will likely
            -- generate them into a new array anyway.
            (\dty res -> if res != -1 then data_types[res] else dty)
            data_types
            resolution
    -- Also, initialize the type resolution vector with result types which gain their type from the node itself.
    let data_types =
        node_types
        |> map production.to_i64
        |> map (\nty -> predetermined_result_types[nty])
        |> map2 (\old new -> if new != data_type.invalid then new else old) data_types
    -- For the other types, which are still invalid, we're going to compute a child which has the same type as this node.
    -- For most operators, we can pick either child, however, for some operators it is preferred to pick the latter child:
    -- - The first child of cast operators is the type.
    -- - The first child of an assignment operator is a reference type.
    -- - The first child of an assignment node might need to be inferred if its an `atom_decl`.
    -- Furthermore, there are two nodes that modifiy the type of its last child: `atom_unary_deref` (which are already
    -- inserted at this point) and `arg`, so we'll need to keep track of whether an inherited type loses a pointer.
    -- We should be able to compute this vector using the logarithmic strategy though.
    -- For arg, we're going to set the inherit order pointer to its resolved param node.
    let data_types =
        -- Build a list of nodes which end the inherit order.
        let ends =
            node_types
            -- The search ends at nodes which are a not an expression...
            |> map production.to_i64
            |> map (\nty -> !is_expr_node[nty])
            -- ... or nodes which have their type already known.
            |> map2
                (||)
                (map (!= data_type.invalid) data_types)
        -- Compute the order in which we're going to search for an inherited type, which is simply a node's last child.
        -- TODO: This code is shared with `build_right_leaf_vector`, maybe it's computation can be shared as well?
        let inherit_order =
            -- First, compute whether this is the last child by scattering (inverting) the prev sibling array.
            scatter
                (replicate n true)
                (map i64.i32 prev_siblings)
                (replicate n false)
            -- Compute a 'last child' vector, by scattering a node's index to the parent _if_ its the last child.
            |> map2 (\parent is_last_child -> if is_last_child then parent else -1) parents
            |> invert
            -- We want to stop looking at nodes which are marked by the `ends` array, so just set their value to -1.
            |> map2 (\end next -> if end then -1 else next) ends
            -- For `arg` nodes, we want to follow the reference pointer instead.
            |> map3
                (\nty res next -> if nty == production_arg then res else next)
                node_types
                resolution
        -- Dereference operators and arg nodes produce a dereferenced version of their arguments.
        let derefs = map (\nty -> nty == production_atom_unary_deref || nty == production_arg) node_types
        let (inherit, loses_reference) =
            resolve_inherits
                inherit_order
                derefs
        -- Finally, fetch and compute the final type.
        in
            inherit
            -- Now, fetch the relevant type.
            |> map (\i -> if i == -1 then data_type.invalid else data_types[i])
            -- Remove the reference if required.
            |> map2 (\remove_ref dty -> if remove_ref then data_type.remove_ref dty else dty) loses_reference
            -- And finally, merge it with the existing data type array. These should all be mutually exclusive valid.
            |> map2 (\old new -> if new != data_type.invalid then new else old) data_types
    -- And finally, we still have one one type of node which doesn't have a type by now: inferenced declarations (atom_decl).
    -- Because we picked the last child when computing the initial type, the type resolution still works. Now that all other
    -- nodes are checked though, we can simply fetch the type from the parent (and add a reference).
    -- Note: The parent of an `atom_decl` is not necessarily `assign`!
    -- TODO: Add a check for that?
    let data_types =
        map3
            (\nty parent dty ->
                -- Accessing parent should be fine, `atom_decl` is never the root node.
                if nty == production_atom_decl && node_types[parent] == production_assign then
                    data_type.add_ref data_types[parent]
                else dty)
            node_types
            parents
            data_types
    in data_types

-- | `resolve_types` computes a result type for every expression node, but doesn't actually verify whether this is
-- consistent with the entire tree. This function performs that check.
let check_types [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data_types: [n]data_type): bool =
    -- In general, data types need to be equal to their parent's type. There are a few exceptions, however, and they fall into
    -- a few different categories:
    -- - Non-expression nodes obviously don't need to be checked, and can be skipped. In principle though, the result
    --   values of these types of nodes should be `invalid`.
    -- - Nodes which are children of non-expression nodes:
    --   - If the parent is `stat_if`, `stat_if_else` or `stat_while`, the result value needs to be an integer.
    --   - If the parent is `stat_return`, the result may be any type that is valid and not a reference.
    --   - If the parent is `stat_expr`, the result may be any valid type.
    -- - No operator accepts `void`, but there are still a few nodes which are allowed to have this value:
    --   `fn_decl`, `atom_stat_expr`, `atom_stat_return`, `atom_fn_call` and `no_expr`.
    --   This and the latter falls under the category 'nodes which type should be equal to their parent's'
    -- - The children of relational operators may be floats and ints (data_type.is_comparable`), but they don't need
    --   to be equal to their parents.
    -- - The left-hand child of an `assign` node should be the ref'ed version of the right-hand child.
    -- - The child of a cast node should be either float or int, and not a reference. (See `data_type.is_castable`.)
    -- - The general case of nodes which' data type should be equal to their parents:
    --   - Any type child of an `arg` node.
    --   - The children of arithmetic non-bitwise operators take both ints and floats (and so their type
    --     must be equal to their parent's).
    --   - The children of operators which only accept ints should take ints (so these must also be
    --     equal to their parent's).
   let check nty parent prev_sibling dty =
        -- This function only handles expression nodes, and so at this point `parent` is guaranteed to be valid.
        let parent_nty = node_types[parent]
        let parent_dty = data_types[parent]
        let is_first_child = prev_sibling == -1
        -- Account for statements of while/if/elif.
        in if parent_nty == production_stat_while || parent_nty == production_stat_if || parent_nty == production_stat_if_else then
            dty == data_type.int
        -- Note: Don't need to check stat_expr for invalid, that's done before this function is called.
        -- Account for return statements.
        else if parent_nty == production_stat_return then
            !(data_type.is_ref dty)
        -- Mostly a sanity check.
        else if parent_nty == production_atom_unary_deref then
            data_type.remove_ref dty == parent_dty
        -- Account for `assign`.
        else if parent_nty == production_assign then
            is_first_child || (data_type.add_ref dty == data_types[prev_sibling])
        -- Account for relational operators.
        else if is_relational_op[production.to_i64 parent_nty] then
            -- Node type should be equal to its sibling, if it has one, and those should also be either ints or floats.
            is_first_child || (data_type.is_comparable dty && dty == data_types[prev_sibling])
        -- Account for casts.
        else if parent_nty == production_atom_cast then
            data_type.is_castable dty parent_dty
        -- Account for void values.
        -- TODO: Is this right? Maybe this construction lets something slip by...
        -- When the child of another node type is one of these and the result is void, it should be caught by those, probably.
        else if dty == data_type.void then
            nty == production_atom_fn_call || nty == production_stat_expr || nty == production_stat_return
            || nty == production_fn_decl || nty == production_no_expr
        -- Filter out nodes who's parents are not expressions. These are things like `fn_decl`, children of `arg_list`s and
        -- children of statement lists.
        -- Just check them explicitly for extra care.
        else if nty == production_fn_decl || parent_nty == production_arg_list || parent_nty == production_param_list
                || nty == production_stat_expr || nty == production_stat_return then
            true
        -- Finally, account for the remaining (arithmetic) nodes.
        -- Note that while the other remaining nodes, like `arg`, are not really arithmetic, they should still pass
        -- this same check, as only those types may be passed to functions anyway.
        else dty == parent_dty
    in
        -- Filter out non-expression nodes and invalid nodes beforehand, to clean up `check` a bit.
        node_types
        |> map production.to_i64
        |> map (\nty -> is_expr_node[nty])
        |> map2
            (==)
            (map (!= data_type.invalid) data_types)
        -- Perform the `check` function for expression type nodes.
        |> map5
            (\nty parent prev_sibling dty valid ->
                if is_expr_node[production.to_i64 nty] then valid && check nty parent prev_sibling dty
                else valid)
            node_types
            parents
            prev_siblings
            data_types
        -- And finally, all of these checks must hold
        |> reduce (&&) true

-- | This function checks whether return statements line up with their function's declared return type.
let check_return_types [n] (node_types: [n]production.t) (parents: [n]i32) (data_types: [n]data_type.t): bool =
    -- First, compute a vector from any node to its function declaration.
    let node_to_decl =
        node_types
        |> map (== production_fn_decl)
        |> map2 (\parent is_fn_decl -> if is_fn_decl then -1 else parent) parents
        |> find_roots
    -- Finally, simply check if the data types are equal for every return statement.
    in
        node_types
        |> map (== production_stat_return)
        |> map3
            (\dty fn_decl is_return ->
                if is_return then fn_decl != -1 && data_types[fn_decl] == dty else true)
            data_types
            node_to_decl
        |> reduce (&&) true
