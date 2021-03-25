import "tree"

let node_type_counts (t: NodeType) : u32 =
    match(t)
        case #invalid -> 0
        case #statement_list -> 0
        case #empty_stat -> 0
        case #func_decl -> 0
        case #expr_stat -> 0
        case #if_stat -> 0 --Insructions handled separately
        case #if_else_stat -> 0 --Instructions handled separately
        case #while_stat -> 0 --Instructions handled separately
        case #func_call_expr -> 0
        case #func_call_arg -> 1
        case #add_expr -> 1
        case #sub_expr -> 1
        case #mul_expr -> 1
        case #div_expr -> 1
        case #mod_expr -> 1
        case #bitand_expr -> 1
        case #bitor_expr -> 1
        case #bitxor_expr -> 1
        case #lshift_expr -> 1
        case #rshift_expr -> 1
        case #urshift_expr -> 1
        case #land_expr -> 0 --Instructions handled separately
        case #lor_expr -> 0 --Instructions handled separately
        case #eq_expr -> 1
        case #neq_expr -> 1
        case #less_expr -> 1
        case #great_expr -> 1
        case #lesseq_expr -> 1
        case #greateq_expr -> 1
        case #bitnot_expr -> 1
        case #lnot_expr -> 1
        case #neg_expr -> 1
        case #lit_expr -> 2
        case #cast_expr -> 1
        case #deref_expr -> 1
        case #assign_expr -> 2
        case #decl_expr -> 1
        case #id_expr -> 1

let node_counts (n: Node) =
    node_type_counts n.node_type

let instr_count_fix (node: Node) (instr_offset: u32) =
    if node.node_type == #invalid then
        0xFFFFFFFF
    else
        instr_offset

let instr_count [max_nodes] (tree: Tree[max_nodes]) =
    map node_counts tree.nodes |>
        scan (+) 0 |>
        rotate (-1) |>
        map2 (\i x -> if i == 0 then 0 else x) (iota max_nodes) |>
        map2 instr_count_fix tree.nodes

let get_function_table [max_nodes] (tree: Tree[max_nodes]) (instr_count: [max_nodes]u32) =
    iota max_nodes |>
        filter (\i -> tree.nodes[i].node_type == #func_decl) |>
        map (\i -> (tree.nodes[i].node_data, instr_count[i])) |>
        unzip2