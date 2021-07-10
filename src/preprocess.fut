import "tree"
import "datatypes"

let INVALID_NODE : Node = {
    node_type = node_type_invalid,
    resulting_type = datatype_invalid,
    parent = 0,
    depth = 0,
    child_idx = 0,
    node_data = 0
}

let is_compare_node (t: NodeType) =
    t >= node_type_eq_expr && t <= node_type_greateq_expr

let copy_node_with_nodetype (n: Node) (t: NodeType) (d: u32) = 
    {
        node_type = t,
        resulting_type = n.resulting_type,
        parent = n.parent,
        depth = n.depth,
        child_idx = n.child_idx,
        node_data = d
    }

let copy_node_with_nodedata (n: Node) (d: u32) = 
    {
        node_type = n.node_type,
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
        if i.parent == INVALID_NODE_IDX then
            (-1i64, INVALID_NODE)
        else if is_compare_node tree.nodes[i.parent].node_type && i.resulting_type == datatype_float then
            (i64.i32 i.parent, copy_node_with_type tree.nodes[i.parent] datatype_float_ref)
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
-- If float and number of int args + float_args - 8 < 8 -> int
-- Else stack

let calling_convention_node_replace_sub (n: Node) (def: NodeType) (fltint: NodeType) (stack: NodeType) =
    if n.resulting_type == datatype_float_ref || n.resulting_type == datatype_float then
        if n.node_data < 8 then
            n
        else
            let num_float_args = i32.u32 n.node_data
            let num_int_args = n.child_idx - num_float_args
            let reg_offset = num_int_args + num_float_args - 8
            in
            if reg_offset < 8 then
                copy_node_with_nodetype n fltint (u32.i32 reg_offset)
            else
                copy_node_with_nodetype n stack (u32.i32 (reg_offset - 8))
    else --Int
        let num_int_args = i32.u32 n.node_data
        let num_float_args = n.child_idx - num_int_args
        let reg_offset = num_int_args + (if num_float_args < 8 then 0 else num_float_args - 8)
        in
        if reg_offset < 8 then
            copy_node_with_nodetype n def (u32.i32 reg_offset)
        else
            copy_node_with_nodetype n stack (u32.i32 (reg_offset - 8))

let calling_convention_node_replace (n: Node) =
    if n.node_type == node_type_func_call_arg then
        calling_convention_node_replace_sub n node_type_func_call_arg node_type_func_call_arg_float_in_int node_type_func_call_arg_stack
    else if n.node_type == node_type_func_arg then
        calling_convention_node_replace_sub n node_type_func_arg node_type_func_arg_float_in_int node_type_func_arg_stack
    else
        n

let replace_arg_types [n] (tree: Tree[n]) =
    {
        nodes = tree.nodes |> map calling_convention_node_replace,
        max_depth = tree.max_depth
    }

let replace_arg_lists [n] (tree: Tree[n]) =
    {
        nodes = iota n |> map (\i ->
                let node = tree.nodes[i] in
                if i > 0 && node.node_type == node_type_func_call_arg_list then
                    let prev_node = tree.nodes[i-1]
                    let num_stack_args = (if prev_node.node_type == node_type_func_call_arg then
                        if prev_node.resulting_type == datatype_float then
                            let total_args = prev_node.child_idx
                            let float_args = i32.u32 prev_node.node_data
                            let int_args = total_args - float_args
                            in
                            u32.i32 (i32.max (int_args-8) 0)
                        else
                            0
                    else if prev_node.node_type == node_type_func_call_arg_stack then
                        prev_node.node_data + 1
                    else
                        0
                    )
                    in
                    copy_node_with_nodedata node num_stack_args
                else
                    node
            ),
        max_depth = tree.max_depth
    }

let preprocess_tree [n] (tree: Tree[n]) =
    tree |> replace_arg_types |> replace_arg_lists |> replace_float_compare_types