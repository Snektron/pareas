import "tree"
import "datatypes"

let node_type_counts (t: NodeType) (d: DataType) : u32 =
    match(t, d)
        case (#invalid, _) -> 0
        case (#statement_list, _) -> 0
        case (#empty_stat, _) -> 0
        case (#func_decl, _) -> 6 --Note: function return
        case (#func_arg, _) -> 1
        case (#func_arg_list, _) -> 0
        case (#expr_stat, _) -> 0
        case (#if_stat, _) -> 0 --Insructions handled separately
        case (#if_else_stat, _) -> 0 --Instructions handled separately
        case (#while_stat, _) -> 1 --Instructions handled separately
        case (#func_call_expr, #void) -> 2
        case (#func_call_expr, _) -> 3
        case (#func_call_arg, _) -> 0 --Handled seperately
        case (#func_call_arg_list, _) -> 0 --Handled separetely
        case (#add_expr, _) -> 1
        case (#sub_expr, _) -> 1
        case (#mul_expr, _) -> 1
        case (#div_expr, _) -> 1
        case (#mod_expr, _) -> 1
        case (#bitand_expr, _) -> 1
        case (#bitor_expr, _) -> 1
        case (#bitxor_expr, _) -> 1
        case (#lshift_expr, _) -> 1
        case (#rshift_expr, _) -> 1
        case (#urshift_expr, _) -> 1
        case (#land_expr, _) -> 0 --Instructions handled separately
        case (#lor_expr, _) -> 0 --Instructions handled separately
        case (#eq_expr, #int) -> 2
        case (#eq_expr, #float) -> 1
        case (#neq_expr, _) -> 2
        case (#less_expr, _) -> 1
        case (#great_expr, _) -> 1
        case (#lesseq_expr, #int) -> 2
        case (#lesseq_expr, #float) -> 1
        case (#greateq_expr, #int) -> 2
        case (#greateq_expr, #float) -> 1
        case (#bitnot_expr, _) -> 1
        case (#lnot_expr, _) -> 1
        case (#neg_expr, _) -> 1
        case (#lit_expr, #int) -> 2
        case (#lit_expr, #float) -> 3
        case (#cast_expr, _) -> 1
        case (#deref_expr, _) -> 1
        case (#assign_expr, _) -> 2
        case (#decl_expr, _) -> 1
        case (#id_expr, _) -> 1
        case (#while_dummy, _) -> 0
        case (#func_decl_dummy, _) -> 6 -- Function stack frame creator
        case (#return_stat, _) -> 2
        case (#func_call_arg_float_in_int, _) -> 0 --Handled separately
        case (#func_call_arg_stack, _) -> 0 -- Handled separately
        case (#func_arg_float_in_int, _) -> 1
        case (#func_arg_stack, _) -> 2
        case (_, _) -> 0

let node_counts (nodes: []Node) (instr_offset: i64) =
    let n = nodes[instr_offset]
    let base_count = node_type_counts n.node_type n.resulting_type
    let delta = (
        if n.node_type == #func_call_arg_list && instr_offset > 0 then
            let prev_node = nodes[instr_offset - 1]
            in
            1 + (if prev_node.node_type == #func_call_arg || prev_node.node_type == #func_call_arg_float_in_int || prev_node.node_type == #func_call_arg_stack then
                let total_args = u32.i32 prev_node.child_idx + 1

                -- let stack_args = u32.i32 (if prev_node.node_type == #func_call_arg_stack then
                --     i32.u32 prev_node.node_data + 1 --If we have a stack node, we know the number of stack args
                -- else if prev_node.node_type == #func_call_arg_float_in_int then
                --     0
                -- else
                --     if prev_node.resulting_type == #float then
                --         let float_args = prev_node.node_data + 1
                --         let int_args = total_args - float_args
                --         in
                --         i32.max ((i32.u32 int_args)-8) 0
                --     else
                --         0
                -- )
                -- let non_stack_args = total_args - stack_args
                -- let prev_node_data = prev_node.node_data
                -- let float_args = if prev_node.resulting_type == #float || prev_node.resulting_type == #float_ref then prev_node_data + 1 else total_args - (prev_node_data + 1)
                -- let int_args = total_args - float_args

                -- let float_reg_args = u32.min float_args 8
                -- let int_reg_args = u32.min (((u32.max float_args 8)-8)+int_args) 8
                -- let non_stack_args = float_reg_args + int_reg_args
                -- let stack_args = total_args - non_stack_args
                in
                --2 * stack_args
                total_args
            else
                0 --No arguments
            )
        else
            0
    )
    + if n.parent == INVALID_NODE_IDX then
        0
    else
        let parent_type = nodes[n.parent].node_type
        in
        if n.child_idx == 0 && (parent_type == #if_stat || parent_type == #if_else_stat) then
            1
        else if n.child_idx == 1 && (parent_type == #if_else_stat  || parent_type == #while_stat) then
            1
        else
            0
    in
    base_count + delta

let instr_count_fix (node: Node) (instr_offset: u32) =
    if node.node_type == #invalid then
        0xFFFFFFFF
    else
        instr_offset

let instr_call_arg_offset (node: Node) =
    let total_args = u32.i32 node.child_idx
    -- let stack_args = u32.i32 (if node.node_type == #func_call_arg_stack then
    --     i32.u32 node.node_data --If we have a stack node, we know the number of stack args
    -- else if node.node_type == #func_call_arg_float_in_int then
    --     0
    -- else
    --     if node.resulting_type == #float then
    --         let float_args = node.node_data
    --         let int_args = total_args - float_args
    --         in
    --         i32.max ((i32.u32 int_args)-8) 0
    --     else
    --         0
    -- )
    -- let non_stack_args = total_args - stack_args

    in
    total_args + 1

let instr_count_fix_post (nodes: []Node) (node_locs: []u32) (node_idx: i64) (instr_offset: u32) =
    let node = nodes[node_idx] in
    if node.parent == INVALID_NODE_IDX then
        (-1i64, 0)
    else
        let parent_type = nodes[node.parent].node_type in
        if node.child_idx == 0 && (parent_type == #if_stat || parent_type == #if_else_stat) then
            (i64.i32 node.parent, instr_offset + node_type_counts node.node_type node.resulting_type)
        else if node.child_idx == 1 && (parent_type == #while_stat) then
            (i64.i32 node.parent, instr_offset + node_type_counts node.node_type node.resulting_type)
        else if node.node_type == #func_call_arg || node.node_type == #func_call_arg_float_in_int || node.node_type == #func_call_arg_stack then
            (node_idx, node_locs[node.parent] + instr_call_arg_offset node)
        else
            (-1i64, 0u32)

let instr_count [max_nodes] (tree: Tree[max_nodes]) =
    let initial_result = map (node_counts tree.nodes) (iota max_nodes) |>
        scan (+) 0 |>
        rotate (-1) |>
        map2 (\i x -> if i == 0 then 0 else x) (iota max_nodes) |>
        map2 instr_count_fix tree.nodes

    let (fix_idx, fix_offsets) =
        map2 (instr_count_fix_post tree.nodes initial_result) (iota max_nodes) initial_result |>
        unzip2
    in
    scatter initial_result fix_idx fix_offsets

let shift_right [n] 't (x: t) (xs: [n]t) : [n]t =
    xs |>
    rotate (-1) |>
    zip (iota n) |>
    map (\(i, y) -> if i == 0 then x else y)

let get_function_table [max_nodes] (tree: Tree[max_nodes]) (instr_count: [max_nodes]u32) =
    let (function_ids, offsets) = iota max_nodes |>
        filter (\i -> tree.nodes[i].node_type == #func_decl) |>
        map (\i -> (tree.nodes[i].node_data, instr_count[i] + 6)) |>
        unzip2
    let rotated_offsets =
        offsets |>
        shift_right 0
    let function_sizes =
        indices function_ids |>
        map (\i -> if i == 0 then offsets[0] else offsets[i] - offsets[i-1])
    in
    (function_ids, rotated_offsets, function_sizes)