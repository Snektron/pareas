import "util"
import "../../../gen/pareas_grammar"

-- | This pass flattens the remaining lists: statement lists, argument lists and function declaration lists.
let flatten_lists [n] (node_types: [n]production.t) (parents: [n]i32): ([n]production.t, [n]i32) =
    let new_node_types =
        map
            (\ty ->
                if ty == production_stat_list_end then production_stat_list
                else if ty == production_fn_decl_list_end then production_fn_decl_list
                else if ty == production_arg_list_end then production_arg_list
                else ty)
            node_types
    let new_parents =
        map2
            (\ty parent ->
                -- Don't need to check whether parent == -1 because production_start is still in the root.
                -- Also don't need to check whether parent points to itself, these nodes are already deleted.
                (ty == production_stat_list && node_types[parent] == production_stat_list)
                || (ty == production_fn_decl_list && node_types[parent] == production_fn_decl_list)
                || (ty == production_arg_list && node_types[parent] == production_arg_list))
            new_node_types
            parents
        -- These lists can be quite long, as functions can have quite a lot of statements in them of course,
        -- so use a logarithmic approach here.
        |> remove_nodes_log parents
    in (new_node_types, new_parents)
