import "util"
import "../../../gen/pareas_grammar"

-- | The list of node types which should be removed in this pass.
local let is_marker = mk_production_mask [
        production_start,
        production_atom_paren,
        production_compound_stat,
        production_stat_compound
    ]

-- | This pass removes 'marker' type nodes, which have no significant semantic information
-- (or this information is only added by making sure that a marked subtree gets placed
-- in the right location in a parent tree). Nodes like parenthesis and compound statements.
-- Nodes like prod, sum etc are already removed in the `fix_bin_ops`. pass.
-- Returns the new parents array.
let remove_marker_nodes [n] (node_types: [n]production.t) (parents: [n]i32): [n]i32 =
    node_types
    |> map production.to_i64
    |> map (\ty -> is_marker[ty])
    -- We expect only a small amount of subsequent nodes need to be removed:
    -- `start` nodes appear only once, and nesting parens/nesting compound expressions
    -- is generally not expected.
    |> remove_nodes_lin parents
