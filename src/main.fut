import "tree"
import "datatypes"
-- let main (n : i64) : u32 =
--     let t = tree.alloc_tree n
--     let result = tree.walk_tree t
--     in result

let MAX_NODES : i64 = 1024

let make_node_type (node_type: u8) : NodeType =
    match node_type
    case 0 -> #invalid
    case 1 -> #statement_list
    case 2 -> #empty_stat
    case 3 -> #func_decl
    case 4 -> #expr_stat
    case 5 -> #if_stat
    case 6 -> #if_else_stat
    case 7 -> #else_aux
    case 8 -> #while_stat
    case 9 -> #func_call_expr
    case 10 -> #func_call_arg
    case 11 -> #add_expr
    case 12 -> #sub_expr
    case 13 -> #mul_expr
    case 14 -> #div_expr
    case 15 -> #mod_expr
    case 16 -> #bitand_expr
    case 17 -> #bitor_expr
    case 18 -> #bitxor_expr
    case 19 -> #lshift_expr
    case 20 -> #rshift_expr
    case 21 -> #urshift_expr
    case 22 -> #land_expr
    case 23 -> #lor_expr
    case 24 -> #eq_expr
    case 25 -> #neq_expr
    case 26 -> #less_expr
    case 27 -> #great_expr
    case 28 -> #lesseq_expr
    case 29 -> #greateq_expr
    case 30 -> #bitnot_expr
    case 31 -> #lnot_expr
    case 32 -> #neg_expr
    case 33 -> #lit_expr
    case 34 -> #cast_expr
    case 35 -> #assign_expr
    case 36 -> #decl_expr
    case 37 -> #id_expr
    case _ -> #invalid

let make_data_type (data_type: u8) : DataType =
    match data_type
    case 0 -> #invalid
    case 1 -> #void
    case 2 -> #int
    case 3 -> #float
    case 4 -> #int_ref
    case 5 -> #float_ref
    case _ -> #invalid

let make_node (node_type: u8) (data_type: u8) (parent: u32) (depth: u32) : Node =
    {
        node_type = make_node_type node_type,
        resulting_type = make_data_type data_type,
        parent = parent,
        depth = depth
    }

entry make_tree (max_depth: u32) (node_types: [MAX_NODES]u8) (data_types: [MAX_NODES]u8) (parents: [MAX_NODES]u32) (depth: [MAX_NODES]u32) : Tree[MAX_NODES] =
    {
        nodes = map4 make_node node_types data_types parents depth,
        max_depth = max_depth
    }

entry main (tree: Tree[MAX_NODES]) =
    i64.u32(tree.max_depth)
