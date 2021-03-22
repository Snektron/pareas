import "tree"
import "datatypes"
import "instr"
import "instr_count"
import "symtab"

let MAX_NODES : i64 = 32
let MAX_VARS : i64 = 32

let make_node_type (node_type: u8) : NodeType =
    match node_type
    case 0 -> #invalid
    case 1 -> #statement_list
    case 2 -> #empty_stat
    case 3 -> #func_decl
    case 4 -> #expr_stat
    case 5 -> #if_stat
    case 6 -> #if_else_stat
    case 7 -> #while_stat
    case 8 -> #func_call_expr
    case 9 -> #func_call_arg
    case 10 -> #add_expr
    case 11 -> #sub_expr
    case 12 -> #mul_expr
    case 13 -> #div_expr
    case 14 -> #mod_expr
    case 15 -> #bitand_expr
    case 16 -> #bitor_expr
    case 17 -> #bitxor_expr
    case 18 -> #lshift_expr
    case 19 -> #rshift_expr
    case 20 -> #urshift_expr
    case 21 -> #land_expr
    case 22 -> #lor_expr
    case 23 -> #eq_expr
    case 24 -> #neq_expr
    case 25 -> #less_expr
    case 26 -> #great_expr
    case 27 -> #lesseq_expr
    case 28 -> #greateq_expr
    case 29 -> #bitnot_expr
    case 30 -> #lnot_expr
    case 31 -> #neg_expr
    case 32 -> #lit_expr
    case 33 -> #cast_expr
    case 34 -> #deref_expr
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


let make_variable (data_type: u8) (global: bool) (offset: u32) : Variable =
    {
        decl_type = make_data_type data_type,
        global = global,
        offset = offset
    }

let make_node (node_type: u8) (data_type: u8, parent: u32, depth: u32, child_idx: u32, node_data: u32) : Node =
    {
        node_type = make_node_type node_type,
        resulting_type = make_data_type data_type,
        parent = parent,
        depth = depth,
        child_idx = child_idx,
        node_data = node_data
    }

entry make_symtab (data_types: [MAX_VARS]u8) (global: [MAX_VARS]bool) (offsets: [MAX_VARS]u32) : Symtab[MAX_VARS] =
    {
        variables = map3 make_variable data_types global offsets
    }

entry make_tree (max_depth: u32) (node_types: [MAX_NODES]u8) (data_types: [MAX_NODES]u8) (parents: [MAX_NODES]u32)
                    (depth: [MAX_NODES]u32) (child_idx: [MAX_NODES]u32) (node_data: [MAX_NODES]u32): Tree[MAX_NODES] =
    {
        nodes = zip5 data_types parents depth child_idx node_data |> map2 make_node node_types,
        max_depth = max_depth
    }

entry make_instr_counts (tree: Tree[MAX_NODES]) =
    instr_count tree

let split_instr (instr: Instr) =
    (instr.instr, instr.rd, instr.rs1, instr.rs2)

entry main (tree: Tree[MAX_NODES]) (symtab: Symtab[MAX_VARS]) (instr_offset: [MAX_NODES]u32) (max_instrs: i64) =
    let instr_offset_i64 = map i64.u32 instr_offset in
    compile_tree tree symtab instr_offset_i64 max_instrs |> map split_instr |> unzip4