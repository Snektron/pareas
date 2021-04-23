import "tree"
import "datatypes"

let INVALID_NODE : Node = {
    node_type = #invalid,
    resulting_type = #invalid,
    parent = 0,
    depth = 0,
    child_idx = 0,
    node_data = 0
}

let is_compare_node (t: NodeType) =
    match t
        case #eq_expr -> true
        case #neq_expr -> true
        case #less_expr -> true
        case #great_expr -> true
        case #lesseq_expr -> true
        case #greateq_expr -> true
        case _ -> false

let copy_node_with_nodetype (n: Node) (t: NodeType) (d: u32) = 
    {
        node_type = t,
        resulting_type = n.resulting_type,
        parent = n.parent,
        depth = n.depth,
        child_idx = n.child_idx,
        node_data = d
    }

let copy_node_with_type (n: Node) (d: DataType) =
    {
        node_type = n.node_type,
        resulting_type = d,
        parent = n.parent,
        depth = n.depth,
        child_idx = n.child_idx,
        node_data = n.node_data
    }

let replace_float_compare_types [n] (tree: Tree[n]) = 
    let (ind, values) = tree.nodes |>
    map (\i ->
        if i.parent == 0xFFFFFFFF then
            (-1i64, INVALID_NODE)
        else if is_compare_node tree.nodes[i64.u32 i.parent].node_type && i.resulting_type == #float then
            (i64.u32 i.parent, copy_node_with_type tree.nodes[i64.u32 i.parent] #float)
        else
            (-1i64, INVALID_NODE)
    ) |>
    unzip2 in

    {
        nodes = scatter (copy tree.nodes) ind values,
        max_depth = tree.max_depth
    }

--Calling convention
-- If float and number of float args < 8 -> float
-- If int and number of int args + max(float_args - 8, 0) < 8 -> int
-- Else stack
let calling_convention_node_replace (n: Node) =
    if n.node_type == #func_call_arg then
        n -- TODO calling convention
    else if n.node_type == #func_arg then
        n -- TODO calling convention
    else
        n

let replace_arg_types [n] (tree: Tree[n]) =
    {
        nodes = tree.nodes |> map calling_convention_node_replace,
        max_depth = tree.max_depth
    }

let preprocess_tree [n] (tree: Tree[n]) =
    tree |> replace_arg_types |> replace_float_compare_types