import "util"
module g = import "../../../gen/pareas_grammar" -- Generated using meson

-- Should be kept in sync with pareas.g
let is_tail_production = mk_production_mask [
        g.production_fn_decl_list_end,

        g.production_stat_list_end,

        g.production_logical_or_list,
        g.production_logical_or_end,

        g.production_logical_and_list,
        g.production_logical_and_end,

        g.production_rela_eq,
        g.production_rela_neq,
        g.production_rela_gt,
        g.production_rela_gte,
        g.production_rela_lt,
        g.production_rela_lte,
        g.production_rela_end,

        g.production_bitwise_and,
        g.production_bitwise_or,
        g.production_bitwise_xor,
        g.production_bitwise_end,

        g.production_shift_lr,
        g.production_shift_ar,
        g.production_shift_ll,
        g.production_shift_end,

        g.production_sum_add,
        g.production_sum_sub,
        g.production_sum_end,

        g.production_prod_mul,
        g.production_prod_div,
        g.production_prod_mod,
        g.production_prod_end
    ]

let is_end_production = mk_production_mask [
        g.production_fn_decl_list_end,
        g.production_stat_list_end,
        g.production_logical_or_end,
        g.production_logical_and_end,
        g.production_rela_end,
        g.production_bitwise_end,
        g.production_shift_end,
        g.production_sum_end,
        g.production_prod_end
    ]

-- Lists have the form
--    sum
--   / \
--  A   sum_add
--     / \
--    B   sum_add
--       / \
--      C   sum_end
-- We are going to remove the ends by transforming the lists into
--    sum_add
--   / \
--  A   sum_add
--     / \
--    C   D
-- This removes redundant list end nodes (marked as invalid after).
-- This operation has a double purpose: consider the tree
--       expr
--       |
--       sum
--      / \
--  prod   end
--    / \
--   id  end
-- This step will clean up the unused prod and sum nodes as well.
let clean_up_lists [n] (nodes: [n]production) (parents: [n]i32) =
    -- Compute the new nodes vector. We do this by moving all of the list tail elements one
    -- element up the tree. Due to the structure of the parse tree, this will never produce a conflict.
    -- First, compute a mask of which nodes will have to move up.
    let tail_nodes = map (\node -> is_tail_production[g.production.to_i64 node]) nodes
    -- Generate the new nodes list simply by scattering. To avoid a filter here, simply set the scatter target
    -- index of a node that shouldn't be moved up to out of bounds, which scatter will ignore for us.
    let is = map2 (\parent is_tail_node -> if is_tail_node then parent else -1) parents tail_nodes |> map i64.i32
    -- Perform the scatter to obtain the new nodes array
    let new_nodes =
        scatter
            (copy nodes)
            is
            nodes
    -- Now, simply mark all list_end nodes and filter them out.
    let remove = map (\node -> is_end_production[g.production.to_i64 node]) new_nodes
    let new_parents = remove_nodes remove parents
    in (new_nodes, new_parents)
