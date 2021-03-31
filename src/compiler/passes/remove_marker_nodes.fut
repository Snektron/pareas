import "util"
import "../../../gen/pareas_grammar"

-- This pass removes 'marker' type nodes, which have no significant semantic information
-- (or this information is only added by making sure that a marked subtree gets placed
-- in the right location in a parent tree). Nodes like parenthesis and compound statements.

-- Nodes like prod, sum etc are already removed in the `clean_lists` pass.

local let is_marker = mk_production_mask [
        production_atom_paren,
        production_compound_stat,
        production_unary_atom
    ]

-- This function simply removes all nodes for which is_marker is true
let remove_marker_nodes [n] (types: [n]production.t) (parents: [n]i32): [n]i32 =
    types
    |> map production.to_i64
    |> map (\ty -> is_marker[ty])
    |> remove_nodes parents
