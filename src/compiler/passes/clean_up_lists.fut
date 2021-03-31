import "util"
import "../../../gen/pareas_grammar"

-- This pass processes 'lists' of grammar items into a more manageable form:
-- list head type productions are removed, and all other productions are
-- shifted one element up the parse tree. Finally, list_end type nodes
-- are removed.

-- Should be kept in sync with pareas.g
local let is_tail_production = mk_production_mask [
        production_fn_decl_list_end,

        production_stat_list_end,

        production_logical_or_list,
        production_logical_or_end,

        production_logical_and_list,
        production_logical_and_end,

        production_rela_eq,
        production_rela_neq,
        production_rela_gt,
        production_rela_gte,
        production_rela_lt,
        production_rela_lte,
        production_rela_end,

        production_bitwise_and,
        production_bitwise_or,
        production_bitwise_xor,
        production_bitwise_end,

        production_shift_lr,
        production_shift_ar,
        production_shift_ll,
        production_shift_end,

        production_sum_add,
        production_sum_sub,
        production_sum_end,

        production_prod_mul,
        production_prod_div,
        production_prod_mod,
        production_prod_end
    ]

local let is_end_production = mk_production_mask [
        production_fn_decl_list_end,
        production_stat_list_end,
        production_logical_or_end,
        production_logical_and_end,
        production_rela_end,
        production_bitwise_end,
        production_shift_end,
        production_sum_end,
        production_prod_end
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
--  prod   sum_end
--    / \
--  id   prod_end
-- Applying the step below will first transform it into
--           expr
--           |
--           sum_end
--          / \
--  prod_end   sum_end
--        / \
--      id   prod_end
-- After pruning all list _ends we get
--  expr
--  |
--  id
let clean_up_lists [n] (nodes: [n]production.t) (parents: [n]i32): ([n]production.t, [n]i32) =
    -- Compute the new nodes vector. We do this by moving all of the list tail elements one
    -- element up the tree. Due to the structure of the parse tree, this will never produce a conflict.
    -- First, compute a mask of which nodes will have to move up.
    let is =
        nodes
        |> map production.to_i64
        |> map (\node -> is_tail_production[node])
        -- Generate the new nodes list simply by scattering. To avoid a filter here, simply set the scatter target
        -- index of a node that shouldn't be moved up to out of bounds, which scatter will ignore for us.
        |> map2 (\parent is_tail_node -> if is_tail_node then parent else -1) parents
        |> map i64.i32
    -- Perform the scatter to obtain the new nodes array
    let new_nodes =
        scatter
            (copy nodes)
            is
            nodes
    -- Now, simply mark all list_end nodes and filter them out.
    let new_parents =
        new_nodes
        |> map production.to_i64
        |> map (\node -> is_end_production[node])
        |> remove_nodes parents
    in (new_nodes, new_parents)
