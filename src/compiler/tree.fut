import "datatypes"

let TREE_SIZE : u32 = 10

--Node types
type NodeType =
    #invalid |
    #statement_list |
    #empty_stat |
    #func_decl |
    #expr_stat |
    #if_stat |
    #if_else_stat |
    #else_aux |
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
    #assign_expr |
    #decl_expr |
    #id_expr

--Node definition
type Node = {
    node_type: NodeType,
    resulting_type: DataType,
    parent: u32,
    depth: u32
}

--Tree definition
type Tree [tree_size] = {
    nodes: [tree_size]Node, --Nodes of the tree
    max_depth: u32 --Tree depth
}

--Type definition for the values of the nodes
type NodeValue = u32 --Temporary type

--Allocates a tree of a given size
let alloc_tree (tree_size: i64) =
    let default_node : Node = {node_type = #invalid, resulting_type = #invalid, parent = 0, depth = 0}
    in {
        nodes = replicate (tree_size) default_node,
        max_depth = 0u32
    }