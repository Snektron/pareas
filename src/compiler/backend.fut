import "codegen/tree"
import "codegen/datatypes"
import "codegen/instr"
import "codegen/instr_count"
import "codegen/register"
import "codegen/preprocess"
import "codegen/optimizer"
import "codegen/postprocess"

type Tree [n] = Tree [n]
type FuncInfo = FuncInfo
type Instr = Instr

let make_node (node_type: u8) (data_type: u8, parent: i32, depth: i32, child_idx: i32, node_data: u32) : Node =
    {
        node_type = i32.u8 node_type,
        resulting_type = i32.u8 data_type,
        parent = parent,
        depth = depth,
        child_idx = child_idx,
        node_data = node_data
    }

let make_functab (id: u32) (start: u32) (size: u32) =
    {
        id = id,
        start = start,
        size = size
    }

-- Data structure rewrite functions
entry make_tree [n] (max_depth: i32) (node_types: [n]u8) (data_types: [n]u8) (parents: [n]i32)
                    (depth: [n]i32) (child_idx: [n]i32) (node_data: [n]u32): Tree[n] =
    {
        nodes = zip5 data_types parents depth child_idx node_data |> map2 make_node node_types,
        max_depth = max_depth
    }

--Stage 1, preprocessor
entry stage_preprocess [n] (tree: Tree[n]) : (Tree[n]) =
    tree |> preprocess_tree

--Stage 2: instruction counting
entry stage_instr_count [n] (tree: Tree[n]) : [n]u32 =
    instr_count tree

--Stage 2.2: function table creation
entry stage_instr_count_make_function_table [n] (tree: Tree[n]) (instr_offset: [n]u32) =
    get_function_table tree instr_offset

entry stage_compact_functab [n] (func_id: [n]u32) (func_start: [n]u32) (func_size: [n]u32) : [n]FuncInfo =
    map3 make_functab func_id func_start func_size

let split_instr (instr: Instr) =
    (instr.instr, instr.rd, instr.rs1, instr.rs2, instr.jt)

--Stage 3: instruction gen
entry stage_instr_gen [n] [k] (tree: Tree[n]) (instr_offset: [n]u32) (func_tab: [k]FuncInfo) : []Instr =
    let func_start = map (.start) func_tab
    let func_size = map (.size) func_tab
    let max_instrs = if n == 0 then 0 else i64.u32 instr_offset[n-1]
    let instr_offset_i64 = map i64.u32 instr_offset
    let func_ends = iota k |> map (\i -> func_start[i] + func_size[i])
    in
    compile_tree tree instr_offset_i64 max_instrs func_start func_ends

let make_instr (instr: u32) (rd: i64) (rs1: i64) (rs2: i64) (jt: u32) =
    {
        instr = instr,
        rd = rd,
        rs1 = rs1,
        rs2 = rs2,
        jt = jt
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

--Stage 4, optimizer
entry stage_optimize [n] [m] (instr_data: [n]Instr) (func_tab: [m]FuncInfo) : ([n]Instr, [m]FuncInfo, [n]bool) =
    -- let instr_data = map5 make_instr instrs rd rs1 rs2 jt
    -- let func_tab = map3 make_functab func_id func_start func_size
    let (instrs, functab, optimize_away) = optimize instr_data func_tab

    -- let (res_instr, res_rd, res_rs1, res_rs2, res_jt) = instrs |> map split_instr |> unzip5
    -- let (res_id, res_start, res_size) = functab |> map split_functab |> unzip3
    in
    (instrs, functab, optimize_away)


--Stage 5,6 regalloc + instr-split
entry stage_regalloc [n] [m] (instrs: [n]Instr) (func_tab: [m]FuncInfo) (func_symbols: [m]u32) (optimize_away: [n]bool) : ([]Instr, [m]FuncInfo) =
    -- let instrs = map5 make_instr instrs rd rs1 rs2 jt
    -- let func_tab = map3 make_functab func_id func_start func_size

    let (instr_offset, lifetime_mask, registers, overflows, swapped, instrs) = (instrs, func_tab, optimize_away, func_symbols) |> register_alloc
    let func_tab = map (fix_func_tab instr_offset) func_tab
    let new_instrs = fill_stack_frames func_tab func_symbols overflows instrs lifetime_mask

    -- let (res_instr, res_rd, res_rs1, res_rs2, res_jt) = new_instrs |> map split_instr |> unzip5
    in
    (new_instrs, func_tab)

--Stage 7: jump fix
entry stage_fix_jumps [n] [m] (instrs: [n]Instr) (func_tab: [m]FuncInfo) : ([]Instr, [m]u32, [m]u32, [m]u32) =
    let (instrs, instr_offset) = instrs |> finalize_jumps
    let func_tab = map (fix_func_tab instr_offset) func_tab

    let (res_id, res_start, res_size) = func_tab |> map split_functab |> unzip3
    in
    (instrs, res_id, res_start, res_size)

-- Stage 8: postprocess
entry stage_postprocess [n] (instrs: [n]Instr) =
    -- let instrs = map5 make_instr instrs rd rs1 rs2 jt

    let result = instrs |> finalize_instr
    let (res_instrs, _, _, _, _) = result |> map split_instr |> unzip5

    in

    res_instrs
