import "util"
import "../../../gen/pareas_grammar"

local let is_list_intermediate = mk_production_mask [
        production_logical_or_list,

        production_logical_and_list,

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
        production_prod_mod
    ]

local let is_list_end = mk_production_mask [
        production_logical_or_end,
        production_logical_and_end,
        production_rela_end,
        production_bitwise_end,
        production_shift_end,
        production_sum_end,
        production_prod_end
    ]

local let expr_list_type = mk_production_array 0i32 [
        (production_logical_or_list, 1),
        (production_logical_or_end, 1),

        (production_logical_and_list, 2),
        (production_logical_and_end, 2),

        (production_rela_eq, 3),
        (production_rela_neq, 3),
        (production_rela_gt, 3),
        (production_rela_gte, 3),
        (production_rela_lt, 3),
        (production_rela_lte, 3),
        (production_rela_end, 3),

        (production_bitwise_and, 4),
        (production_bitwise_xor, 4),
        (production_bitwise_or, 4),
        (production_bitwise_end, 4),

        (production_shift_lr, 5),
        (production_shift_ar, 5),
        (production_shift_ll, 5),
        (production_shift_end, 5),

        (production_sum_add, 6),
        (production_sum_sub, 6),
        (production_sum_end, 6),

        (production_prod_mul, 7),
        (production_prod_div, 7),
        (production_prod_mod, 7),
        (production_prod_end, 7)
    ]

let fix_bin_ops [n] (nodes: [n]production.t) (parents: [n]i32) =
    -- First, move all the parent pointers of nodes that point to list intermediates one up.
    let new_parents =
        iota n
        |> map
            (\i ->
                !is_list_end[production.to_i64 nodes[i]]
                && !is_list_intermediate[production.to_i64 nodes[i]]
                && parents[i] != -1
                && is_list_intermediate[production.to_i64 nodes[parents[i]]])
        |> map2
            (\parent im -> if im then parents[parent] else parent)
            parents
        -- Also remove old list ends - these should have no children so removing them should be cheap
        |> map2
            (\i parent -> if is_list_end[production.to_i64 nodes[i]] then i else parent)
            (iota n |> map i32.i64)
    -- Compute new nodes by moving all list intermediate/end nodes one up.
    let new_nodes =
        let is =
            nodes
            |> map production.to_i64
            |> map (\node -> is_list_intermediate[node] || is_list_end[node])
            -- Generate the new nodes list simply by scatterin To avoid a filter here, simply set the scatter target
            -- index of a node that shouldn't be moved up to out of bounds, which scatter will ignore for us.
            --
            -- It does not really matter to use the new or old parents array here.
            |> map2 (\parent is_tail_node -> if is_tail_node then parent else -1) parents
            |> map i64.i32
        in scatter
            (copy nodes)
            is
            nodes
    let expr_type = map (\node -> expr_list_type[production.to_i64 node]) new_nodes
    -- Compute whether this node's expression type is the same as that of the parent - and thus whether their
    -- pointers need to be flipped.
    let same_type_as_parent =
        iota n
        |> map i32.i64
        -- Also remove those old list ends here so we don't get any problems down the line.
        |> map (\i -> new_parents[i] != i && new_parents[i] != -1 && expr_type[i] != 0 && expr_type[i] == expr_type[new_parents[i]])
    -- Make the parent of each of the list end nodes the parent of the entire list.
    -- Do this simply by marking the same_type_as_parent nodes, computing the parents, and computing the parents
    -- of those again.
    let new_parents =
        let end_parents = find_unmarked_parents new_parents same_type_as_parent
        in
            iota n
            -- Careful to not mess up lists that only have the end node here
            |> map2 (\node i -> is_list_end[production.to_i64 node] && same_type_as_parent[i]) new_nodes
            |> map3
                (\parent end_parent is_end -> if is_end then new_parents[end_parent] else parent)
                new_parents
                end_parents
    -- Invert the lists by, for each of the same_type_as_parents node, scatter to their _original_ parent,
    -- This step relies on that the old list ends have been removed, which is why they are filtered
    -- out during construction of `same_type_as_parent`.
    let new_parents =
        let is = map2 (\parent same_type -> if same_type then parent else -1) parents same_type_as_parent
        in
            scatter
                (copy new_parents)
                (is |> map i64.i32)
                (iota n |> map i32.i64)
    -- Finally, just remove all the list end markers.
    let new_parents =
        new_nodes
        |> map production.to_i64
        |> map (\node -> is_list_end[node])
        |> remove_nodes new_parents
    in (new_nodes, new_parents)
