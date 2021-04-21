import "util"
import "../util"
import "../datatypes"
import "../../../gen/pareas_grammar"

-- | A mask for all nodes that should gain a result type, which in general are all expression nodes,
-- along with `stat_expr` and `stat_return`.
-- Note that list end and head nodes are already filtered out, as well as a few other nodes.
local let is_expr_node = mk_production_mask [
        production_stat_expr,
        production_stat_return,
        production_assign,
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
        -- production_atom_fn_proto, -- Not relevant for this pass.
        production_atom_decl,
        production_atom_unary_deref,
        production_arg
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
        (production_rela_lte, data_type.int)
    ]

-- | Translate a node type (such as type_int) into a data type. If its not a `type`-type node,
-- the invalid data type is returned.
local let node_type_to_data_type (nt: production.t): data_type =
    if nt == production_type_void then data_type.void
    else if nt == production_type_int then data_type.int
    else if nt == production_type_float then data_type.float
    else data_type.invalid

-- | A small helper function that checks if a node produces a reference type.
local let node_produces_reference (nt: production.t): bool =
    nt == production_atom_decl || nt == production_atom_name

-- | This function resolves inherits by following linked-list pointer chains, skipping nodes marked in `skip`
-- in the process. This function is mostly the same as `find_unmarked_parents_log`, except that it also checks
-- whether a pointer passed a dereference-type node (given by `is_deref`).
local let resolve_inherits [n] (parents: [n]i32) (skip: [n]bool) (is_deref: [n]bool): ([n]i32, [n]bool) =
    iterate
        (n |> i32.i64 |> bit_width)
        (\(links, loses_reference) ->
            let loses_reference' =
                links
                |> map (\link -> link != -1 && loses_reference[link])
                |> map2 (||) loses_reference
            let links' =
                map
                    (\link -> if link == -1 || !skip[link] then link else links[link])
                    links
            in (links', loses_reference'))
        (parents, is_deref)

-- | This pass resolves (but not checks!) a type for each expression-type node.
let resolve_types [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (resolution: [n]i32) =
    -- Initialize the type resolution vector with nodes which inherit their results from their 'type' children.
    -- These include (function prototype) declarations and cast nodes.
    let data_types =
        -- Compute the data type according to the type and it's parent (which might add a reference).
        let vs =
            node_types
            -- Map these data types to their initial types
            |> map node_type_to_data_type
            -- Add a reference for `atom_decl` and `atom_name` nodes.
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
    -- This includes argument (`arg` nodes) types, function return types, and `atom_name` types.
    -- We can simply fetch decl and name nodes, but argument nodes lose their reference again.
    let data_types =
        map2
            -- This should be fine to do in-place as they are disjoint, but futhark will likely
            -- generate them into a new array anyway.
            (\dty res -> if res != -1 then data_types[res] else dty)
            data_types
            resolution
        |> map2 (\nty dty -> if nty == production_arg then data_type.remove_ref dty else dty) node_types
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
    -- Furthermore, there is one node that modifies the type of its last child: `atom_unary_deref` (which are already
    -- inserted at this point), so we'll need to keep track of whether an inherited type loses a pointer.
    -- We should be able to compute this vector using the logarithmic strategy though.
    let data_types =
        -- Compute a mask for nodes we want to skip: Those which are currently invalid.
        let mask =
            data_types
            |> map (== data_type.invalid)
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
        let (inherit, loses_reference) =
            resolve_inherits
                inherit_order
                mask
                (map (== production_atom_unary_deref) node_types)
        -- Build a quick mask of nodes which appear in an expression
        let is_expr =
            node_types
            |> map production.to_i64
            |> map (\nty -> is_expr_node[nty])
        -- Finally, fetch and compute the final type.
        in
            inherit
            -- We don't care about non-expression nodes and nodes which already had a value, so ignore those.
            |> map2 (\in_expr i -> if in_expr then i else -1) is_expr
            -- We also don't care about nodes which already had a valid datatype.
            |> map2 (\dty i -> if dty == data_type.invalid then i else -1) data_types
            -- Now, fetch the relevant type.
            |> map (\i -> if i == -1 then data_type.invalid else data_types[i])
            -- Remove the reference if required.
            |> map2 (\remove_ref dty -> if remove_ref then data_type.remove_ref dty else dty) loses_reference
            -- And finally, merge it with the existing data type array. These should all be mutually exclusive valid.
            |> map2 (\old new -> if new != data_type.invalid then new else old) data_types
    in data_types
