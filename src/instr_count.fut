import "tree"
import "datatypes"

let node_type_counts (t: NodeType) (d: DataType) : u32 =
    match(t, d)
        case (#invalid, _) -> 0
        case (#statement_list, _) -> 0
        case (#empty_stat, _) -> 0
        case (#func_decl, _) -> 0
        case (#expr_stat, _) -> 0
        case (#if_stat, _) -> 0 --Insructions handled separately
        case (#if_else_stat, _) -> 0 --Instructions handled separately
        case (#while_stat, _) -> 0 --Instructions handled separately
        case (#func_call_expr, _) -> 0
        case (#func_call_arg, _) -> 1
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
        case (#lit_expr, _) -> 2
        case (#cast_expr, _) -> 1
        case (#deref_expr, _) -> 1
        case (#assign_expr, _) -> 2
        case (#decl_expr, _) -> 1
        case (#id_expr, _) -> 1
        case (_, _) -> 0

let node_counts (n: Node) =
    node_type_counts n.node_type n.resulting_type

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

let shift_right [n] 't (x: t) (xs: [n]t) : [n]t =
    xs |>
    rotate (-1) |>
    zip (iota n) |>
    map (\(i, y) -> if i == 0 then x else y)

let get_function_table [max_nodes] (tree: Tree[max_nodes]) (instr_count: [max_nodes]u32) =
    let (function_ids, offsets) = iota max_nodes |>
        filter (\i -> tree.nodes[i].node_type == #func_decl) |>
        map (\i -> (tree.nodes[i].node_data, instr_count[i])) |>
        unzip2
    let rotated_offsets =
        offsets |>
        shift_right 0
    let function_sizes =
        indices function_ids |>
        map (\i -> if i == 0 then offsets[0] else offsets[i] - offsets[i-1])
    in
    (function_ids, rotated_offsets, function_sizes)