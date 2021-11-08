import "datatypes"

let INVALID_NODE_IDX : i32 = -1

--Node types
type NodeType = i32

let node_type_invalid : i32 = 0
let node_type_statement_list : i32 = 1
let node_type_empty_stat : i32 = 2
let node_type_func_decl : i32 = 3
let node_type_func_arg : i32 = 4
let node_type_func_arg_list : i32 = 5
let node_type_expr_stat : i32 = 6
let node_type_if_stat : i32 = 7
let node_type_if_else_stat : i32 = 8
let node_type_while_stat : i32 = 9
let node_type_func_call_expr : i32 = 10
let node_type_func_call_arg : i32 = 11
let node_type_func_call_arg_list : i32 = 12
let node_type_add_expr : i32 = 13
let node_type_sub_expr : i32 = 14
let node_type_mul_expr : i32 = 15
let node_type_div_expr : i32 = 16
let node_type_mod_expr : i32 = 17
let node_type_bitand_expr : i32 = 18
let node_type_bitor_expr : i32 = 19
let node_type_bitxor_expr : i32 = 20
let node_type_lshift_expr : i32 = 21
let node_type_rshift_expr : i32 = 22
let node_type_urshift_expr : i32 = 23
let node_type_land_expr : i32 = 24
let node_type_lor_expr : i32 = 25
let node_type_eq_expr : i32 = 26
let node_type_neq_expr : i32 = 27
let node_type_less_expr : i32 = 28
let node_type_great_expr : i32 = 29
let node_type_lesseq_expr : i32 = 30
let node_type_greateq_expr : i32 = 31
let node_type_bitnot_expr : i32 = 32
let node_type_lnot_expr : i32 = 33
let node_type_neg_expr : i32 = 34
let node_type_lit_expr : i32 = 35
let node_type_cast_expr : i32 = 36
let node_type_deref_expr : i32 = 37
let node_type_assign_expr : i32 = 38
let node_type_decl_expr : i32 = 39
let node_type_id_expr : i32 = 40
let node_type_while_dummy : i32 = 41
let node_type_func_decl_dummy : i32 = 42
let node_type_return_stat : i32 = 43
let node_type_func_call_arg_float_in_int : i32 = 44
let node_type_func_call_arg_stack : i32 = 45
let node_type_func_arg_float_in_int : i32 = 46
let node_type_func_arg_stack : i32 = 47

--Node definition
type Node = {
    node_type: NodeType,
    resulting_type: DataType,
    parent: i32,
    depth: i32,
    child_idx: i32,
    node_data: u32
}

--Tree definition
type Tree [tree_size] = {
    nodes: [tree_size]Node, --Nodes of the tree
    max_depth: i32 --Tree depth
}

let is_level (n : Node) (depth: i32) =
    n.depth == depth
