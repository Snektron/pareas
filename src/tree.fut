import "datatypes"

--Node types
type NodeType =
    #invalid |
    #statement_list |
    #empty_stat |
    #func_decl |
    #expr_stat |
    #if_stat |
    #if_else_stat |
    #while_stat |
    #func_call_expr |
    #func_call_arg |
    #add_expr |
    #sub_expr |
    #mul_expr |
    #div_expr |
    #mod_expr |
    #bitand_expr |
    #bitor_expr |
    #bitxor_expr |
    #lshift_expr |
    #rshift_expr |
    #urshift_expr |
    #land_expr |
    #lor_expr |
    #eq_expr |
    #neq_expr |
    #less_expr |
    #great_expr |
    #lesseq_expr |
    #greateq_expr |
    #bitnot_expr |
    #lnot_expr |
    #neg_expr |
    #lit_expr |
    #cast_expr |
    #deref_expr |
    #assign_expr |
    #decl_expr |
    #id_expr

--Node definition
type Node = {
    node_type: NodeType,
    resulting_type: DataType,
    parent: u32,
    depth: u32,
    child_idx: u32,
    node_data: u32
}

--Tree definition
type Tree [tree_size] = {
    nodes: [tree_size]Node, --Nodes of the tree
    max_depth: u32 --Tree depth
}

let is_level (n : Node) (depth: u32) =
    n.depth == depth