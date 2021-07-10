import "tree"
import "datatypes"
import "instr"
import "instr_count"
import "symtab"
import "register"
import "preprocess"
import "optimizer"
import "postprocess"

let make_variable (data_type: u8) (offset: u32) : Variable =
    {
        decl_type = i32.u8 data_type,
        offset = offset
    }

let make_node (node_type: u8) (data_type: u8, parent: i32, depth: i32, child_idx: i32, node_data: u32) : Node =
    {
        node_type = i32.u8 node_type,
        resulting_type = i32.u8 data_type,
        parent = parent,
        depth = depth,
        child_idx = child_idx,
        node_data = node_data
    }

entry make_symtab [m] (data_types: [m]u8) (offsets: [m]u32) : Symtab[m] =
    {
        variables = map2 make_variable data_types offsets
    }

--Stage 1, preprocessor
entry stage_preprocess [n] (max_depth: i32) (node_types: [n]u8) (data_types: [n]u8) (parents: [n]i32)
                    (depth: [n]i32) (child_idx: [n]i32) (node_data: [n]u32): Tree[n] =
    {
        nodes = zip5 data_types parents depth child_idx node_data |> map2 make_node node_types,
        max_depth = max_depth
    }
    |> preprocess_tree

--Stage 2: instruction counting
entry stage_instr_count [n] (tree: Tree[n]) =
    instr_count tree

--Stage 2.2: function table creation
entry make_function_table [n] (tree: Tree[n]) (instr_offset: [n]u32) =
    get_function_table tree instr_offset

let split_instr (instr: Instr) =
    (instr.instr, instr.rd, instr.rs1, instr.rs2, instr.jt)

--Stage 3: instruction gen
entry stage_instr_gen [n] [m] [k] (tree: Tree[n]) (symtab: Symtab[m]) (instr_offset: [n]u32) (func_start: [k]u32) (func_size: [k]u32) =
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

let split_functab (func_info: FuncInfo) =
    (func_info.id, func_info.start, func_info.size)

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

entry stage_optimize [n] [m] (instrs: [n]u32) (rd: [n]i64) (rs1: [n]i64) (rs2: [n]i64) (jt: [n]u32) (func_id: [m]u32) (func_start: [m]u32) (func_size: [m]u32) =
    let instr_data = map5 make_instr instrs rd rs1 rs2 jt
    let func_tab = map3 make_functab func_id func_start func_size
    let (instrs, functab, optimize_away) = optimize instr_data func_tab

    let (res_instr, res_rd, res_rs1, res_rs2, res_jt) = instrs |> map split_instr |> unzip5
    let (res_id, res_start, res_size) = functab |> map split_functab |> unzip3
    in
    (res_instr, res_rd, res_rs1, res_rs2, res_jt, res_id, res_start, res_size, optimize_away)


entry stage_regalloc [n] [m] (instrs: [n]u32) (rd: [n]i64) (rs1: [n]i64) (rs2: [n]i64) (jt: [n]u32) (func_id: [m]u32) (func_start: [m]u32) (func_size: [m]u32) (func_symbols: [m]u32) (optimize_away: [n]bool) =
    let instrs = map5 make_instr instrs rd rs1 rs2 jt
    let func_tab = map3 make_functab func_id func_start func_size

    let (instr_offset, lifetime_mask, registers, overflows, swapped, instrs) = (instrs, func_tab, optimize_away, func_symbols) |> register_alloc
    let func_tab = map (fix_func_tab instr_offset) func_tab
    let new_instrs = fill_stack_frames func_tab func_symbols overflows instrs lifetime_mask
    
    let (res_instr, res_rd, res_rs1, res_rs2, res_jt) = new_instrs |> map split_instr |> unzip5
    let (res_id, res_start, res_size) = func_tab |> map split_functab |> unzip3
    in
    (res_instr, res_rd, res_rs1, res_rs2, res_jt, res_id, res_start, res_size)

entry stage_postprocess [n] (instrs: [n]u32) (rd: [n]i64) (rs1: [n]i64) (rs2: [n]i64) (jt: [n]u32) =
    let instrs = map5 make_instr instrs rd rs1 rs2 jt

    let result = instrs |> finalize_instr
    let (res_instrs, _, _, _, _) = result |> map split_instr |> unzip5

    in

    res_instrs