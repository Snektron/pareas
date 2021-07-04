import "tree"
import "datatypes"
import "instr"
import "instr_count"
import "symtab"
import "register"
import "preprocess"
import "optimizer"
import "postprocess"

let make_node_type (node_type: u8) : NodeType =
    match node_type
    case 0 -> #invalid
    case 1 -> #statement_list
    case 2 -> #empty_stat
    case 3 -> #func_decl
    case 4 -> #func_arg
    case 5 -> #func_arg_list
    case 6 -> #expr_stat
    case 7 -> #if_stat
    case 8 -> #if_else_stat
    case 9 -> #while_stat
    case 10 -> #func_call_expr
    case 11 -> #func_call_arg
    case 12 -> #func_call_arg_list
    case 13 -> #add_expr
    case 14 -> #sub_expr
    case 15 -> #mul_expr
    case 16 -> #div_expr
    case 17 -> #mod_expr
    case 18 -> #bitand_expr
    case 19 -> #bitor_expr
    case 20 -> #bitxor_expr
    case 21 -> #lshift_expr
    case 22 -> #rshift_expr
    case 23 -> #urshift_expr
    case 24 -> #land_expr
    case 25 -> #lor_expr
    case 26 -> #eq_expr
    case 27 -> #neq_expr
    case 28 -> #less_expr
    case 29 -> #great_expr
    case 30 -> #lesseq_expr
    case 31 -> #greateq_expr
    case 32 -> #bitnot_expr
    case 33 -> #lnot_expr
    case 34 -> #neg_expr
    case 35 -> #lit_expr
    case 36 -> #cast_expr
    case 37 -> #deref_expr
    case 38 -> #assign_expr
    case 39 -> #decl_expr
    case 40 -> #id_expr
    case 41 -> #while_dummy
    case 42 -> #func_decl_dummy
    case 43 -> #return_stat
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


let make_variable (data_type: u8) (offset: u32) : Variable =
    {
        decl_type = make_data_type data_type,
        offset = offset
    }

let make_node (node_type: u8) (data_type: u8, parent: i32, depth: i32, child_idx: i32, node_data: u32) : Node =
    {
        node_type = make_node_type node_type,
        resulting_type = make_data_type data_type,
        parent = parent,
        depth = depth,
        child_idx = child_idx,
        node_data = node_data
    }

entry make_symtab [m] (data_types: [m]u8) (offsets: [m]u32) : Symtab[m] =
    {
        variables = map2 make_variable data_types offsets
    }

entry make_tree [n] (max_depth: i32) (node_types: [n]u8) (data_types: [n]u8) (parents: [n]i32)
                    (depth: [n]i32) (child_idx: [n]i32) (node_data: [n]u32): Tree[n] =
    {
        nodes = zip5 data_types parents depth child_idx node_data |> map2 make_node node_types,
        max_depth = max_depth
    } |> preprocess_tree

entry make_instr_counts [n] (tree: Tree[n]) =
    instr_count tree

entry make_function_table [n] (tree: Tree[n]) (instr_offset: [n]u32) =
    get_function_table tree instr_offset

let split_instr (instr: Instr) =
    (instr.instr, instr.rd, instr.rs1, instr.rs2, instr.jt)

entry main [n] [m] [k] (tree: Tree[n]) (symtab: Symtab[m]) (instr_offset: [n]u32) (func_start: [k]u32) (func_size: [k]u32) =
    let max_instrs = if n == 0 then 0 else i64.u32 instr_offset[n-1]
    let instr_offset_i64 = map i64.u32 instr_offset
    let func_ends = iota k |> map (\i -> func_start[i] + func_size[i])
    in
    compile_tree tree symtab instr_offset_i64 max_instrs func_start func_ends |> map split_instr |> unzip5

let make_instr (instr: u32) (rd: i64) (rs1: i64) (rs2: i64) (jt: u32) =
    {
        instr = instr,
        rd = rd,
        rs1 = rs1,
        rs2 = rs2,
        jt = jt
    }

let make_functab (id: u32) (start: u32) (size: u32) =
    {
        id = id,
        start = start,
        size = size
    }

let fix_func_tab [n] (instr_offsets: [n]i32) (func_info: FuncInfo) =
    let func_start = u32.i32 instr_offsets[i64.u32 func_info.start]
    let func_end_loc = i64.u32 (func_info.start + func_info.size)
    let func_end = if func_end_loc >= n then u32.i32 instr_offsets[n-1]+1 else u32.i32 instr_offsets[func_end_loc]
    let func_size = func_end - func_start
    in
    {
        id = func_info.id,
        start = func_start,
        size = func_size
    }

entry do_register_alloc [n] [m] (instrs: [n]u32) (rd: [n]i64) (rs1: [n]i64) (rs2: [n]i64) (jt: [n]u32) (func_id: [m]u32) (func_start: [m]u32) (func_size: [m]u32) (func_symbols: [m]u32) =
    let instr_data = map5 make_instr instrs rd rs1 rs2 jt
    let func_tab = map3 make_functab func_id func_start func_size
    let (instrs, functab, optimize_away) = optimize instr_data func_tab
    let (instr_offset, lifetime_mask, registers, overflows, swapped, instrs) = (instrs, functab, optimize_away, func_symbols) |> register_alloc
    let func_tab = map (fix_func_tab instr_offset) func_tab
    let new_instrs = fill_stack_frames func_tab func_symbols overflows instrs lifetime_mask |> finalize_instr
    --let new_instrs = instrs
    in
    (instr_offset, lifetime_mask, registers, optimize_away, new_instrs |> map (\i -> i.instr), swapped,
        new_instrs |> map (.rd), new_instrs |> map (.rs1), new_instrs |> map (.rs2), new_instrs |> map (.jt))