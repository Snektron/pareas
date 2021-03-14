import "tree"
import "datatypes"
import "symtab"
import "../lib/github.com/diku-dk/sorts/radix_sort"

type Instr = {
    instr: u32,
    rd: i64,
    rs1: i64,
    rs2: i64
}

let EMPTY_INSTR : Instr = {
    instr = 0,
    rd = 0,
    rs1 = 0,
    rs2 = 0
}

let PARENT_IDX_PER_NODE : i64 = 2

let node_instr(node_type: NodeType) (data_type: DataType) (instr_offset: i64) : u32 =
    match (node_type, data_type, instr_offset)
    --Language constructs without instruction
    case (#statement_list, _, 0) ->     0b0000000_00000_00000_000_00000_0000000 -- ignored
    case (#empty_stat, _, 0) ->         0b0000000_00000_00000_000_00000_0000000 -- ignored
    case (#expr_stat, _, 0) ->          0b0000000_00000_00000_000_00000_0000000 -- ignored

    -- Scope control
    case (#func_decl, _, _) ->          0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Control flow
    case (#if_stat, _, _) ->            0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#if_else_stat, _, _) ->       0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#else_aux, _, _) ->           0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#while_stat, _, _) ->         0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Binary integer arithmetic
    case (#add_expr, #int, 0) ->        0b0000000_00000_00000_000_00000_0110011 -- ADD
    case (#sub_expr, #int, 0) ->        0b0100000_00000_00000_000_00000_0110011 -- SUB
    case (#mul_expr, #int, 0) ->        0b0000001_00000_00000_000_00000_0110011 -- MUL
    case (#div_expr, #int, 0) ->        0b0000001_00000_00000_100_00000_0110011 -- DIV
    case (#mod_expr, #int, 0) ->        0b0000001_00000_00000_110_00000_0110011 -- REM
    case (#bitand_expr, #int, 0) ->     0b0000000_00000_00000_111_00000_0110011 -- AND
    case (#bitor_expr, #int, 0) ->      0b0000000_00000_00000_110_00000_0110011 -- OR
    case (#bitxor_expr, #int, 0) ->     0b0000000_00000_00000_100_00000_0110011 -- XOR
    case (#lshift_expr, #int, 0) ->     0b0000000_00000_00000_001_00000_0110011 -- SLL
    case (#rshift_expr, #int, 0) ->     0b0100000_00000_00000_101_00000_0110011 -- SRA
    case (#urshift_expr, #int, 0) ->    0b0000000_00000_00000_101_00000_0110011 -- SRL

    -- Binary float arithmetic
    case (#add_expr, #float, 0) ->      0b0000000_00000_00000_111_00000_1010011 -- FADD.S
    case (#sub_expr, #float, 0) ->      0b0000100_00000_00000_111_00000_1010011 -- FSUB.S
    case (#mul_expr, #float, 0) ->      0b0001000_00000_00000_111_00000_1010011 -- FMUL.S
    case (#div_expr, #float, 0) ->      0b0001100_00000_00000_111_00000_1010011 -- FDIV.S

    -- Logical nodes
    case (#land_expr, #int, _) ->       0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#lor_expr, #int, _) ->        0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Unary integer arithmetic
    case (#bitnot_expr, #int, 0) ->     0b1111111_11111_00000_100_00000_0010011 -- XORI -1
    case (#lnot_expr, #int, 0) ->       0b0000000_00001_00000_011_00000_0010011 -- SLTUI 1
    case (#neg_expr, #int, 0) ->        0b0100000_00000_00000_000_00000_0110011 -- SUB ri, x0

    -- Unary float arithmetic
    case (#neg_expr, #float, 0) ->      0b0010000_00000_00000_001_00000_1010011 -- FSGNJN.S (ensure same operand twice)

    -- Integer comparision
    case (#eq_expr, #int, _) ->         0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#neq_expr, #int, 0) ->        0b0100000_00000_00000_000_00000_0110011 -- SUB
    case (#less_expr, #int, _) ->       0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#great_expr, #int, _) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#lesseq_expr, #int, _) ->     0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#greateq_expr, #int, _) ->    0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Float comparision: TODO, change this lookup, return type is int
    case (#eq_expr, #float, _) ->       0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#neq_expr, #float, _) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#less_expr, #float, _) ->     0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#great_expr, #float, _) ->    0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#lesseq_expr, #float, _) ->   0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#greateq_expr, #float, _) ->  0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Assignment
    case (#assign_expr, #int_ref, 0) -> 0b0000000_00000_00000_010_00000_0100011 -- SW offset 0
    case (#assign_expr, #int_ref, 1) -> 0b0000000_00000_00000_000_00000_0110011 -- ADD x0

    -- Function call
    case (#func_call_expr, _, _) ->     0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#func_call_arg, _, _) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Literals
    case (#lit_expr, #int, 0) ->        0b0000000_00000_00000_000_00000_0110111 -- LUI
    case (#lit_expr, #int, 1) ->        0b0000000_00000_00000_110_00000_0010011 -- ORI
    case (#lit_expr, #float, 0) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#lit_expr, #float, 1) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Casts (NOTE: filter out int->int or float->float casts in earlier stages, done with single map in preprocessing stage)
    case (#cast_expr, #int, 0) ->       0b1100000_00000_00000_111_00000_1010011 -- FCVT.W.S
    case (#cast_expr, #float, 0) ->     0b1101000_00000_00000_111_00000_1010011 -- FCVT.S.W
    case (#deref_expr, #int, 0) ->      0b0000000_00000_00000_010_00000_0000011 -- LW 0(rs1)
    case (#deref_expr, #float, 0) ->    0b0000000_00000_00000_010_00000_0000111 -- FLW 0(rs1)

    -- Variables, TODO add global support using symbol table symbol id
    case (#decl_expr, _, 0) ->          0b0000000_00000_01000_000_00000_0010011 -- ADDI bp (local offsets inverted)
    case (#id_expr, _, 0) ->            0b0000000_00000_01000_000_00000_0010011 -- ADDI bp (local offsets inverted)

    --Invalid opcodes
    case _ ->                           0b0000000_00000_00000_000_00000_1110011 -- EBREAK

let has_instr (node_type: NodeType) (instr_offset: i64) : bool =
    match(node_type, instr_offset)
        case (#lit_expr, 1) -> true
        case (#assign_expr, 1) -> true
        case (_, 0) -> true
        case _ -> false

let node_has_return(_ : NodeType) (data_type : DataType) : bool =
    data_type != #void

let parent_arg_idx (node: Node) : i64 =
    i64.u32 node.parent * PARENT_IDX_PER_NODE + i64.u32 node.child_idx

let node_get_parent_arg_idx (node: Node) (instr_offset: i64) : i64 =
    match (node.node_type, instr_offset)
        case (#lit_expr, 0) -> -1
        case (#assign_expr, 0) -> -1
        case (_, 0) ->
            if node_has_return node.node_type node.resulting_type then
                parent_arg_idx node
            else
                -1
        case (#lit_expr, 1) -> parent_arg_idx node
        case (#assign_expr, 1) -> parent_arg_idx node
        case _ ->
            -1

let register (instr_no: i64) =
    instr_no + 32

let node_get_instr_arg (node_id: i64) (node: Node) (registers: []i64) (arg_no: i64) (instr_no: i64) (instr_offset: i64) : i64 =
    match(node.node_type, arg_no, instr_offset)
        case (#add_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#sub_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#mul_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#div_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#mod_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#bitand_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#bitor_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#bitxor_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#lshift_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#rshift_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#urshift_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#assign_expr, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#assign_expr, 0, 1) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#cast_expr, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#deref_expr, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]

        case (#lit_expr, 0, 1) -> register (instr_no - 1)
        case _ -> 0

let node_has_output (node: Node) (instr_offset: i64) : bool =
    (node_get_parent_arg_idx node instr_offset) != -1 || match (node.node_type, instr_offset)
        case (#lit_expr, 0) -> true
        case _ -> false


let instr_constant [max_vars] (node: Node) (instr_offset: i64) (symtab: Symtab[max_vars]) : u32 =
    match(node.node_type, node.resulting_type, instr_offset)
        case (#lit_expr, #int, 0) -> node.node_data & 0xFFFFF000
        case (#lit_expr, #int, 1) -> (node.node_data & 0xFFF) << 20
        case (#id_expr, _, 0) -> (-4 * ((symtab_local_offset symtab node.node_data) + 1)) << 20
        case (#decl_expr, _, 0) -> (-4 * ((symtab_local_offset symtab node.node_data) + 1)) << 20
        case _ -> 0

let get_node_instr [max_vars] (node: Node) (instr_no: i64) (node_index: i64) (registers: []i64) (symtab: Symtab[max_vars]) (instr_offset: i64): (i64, i64, Instr) =
    let node_type = node.node_type
    let data_type = node.resulting_type
    in
        (
            instr_no,
            node_get_parent_arg_idx node instr_offset,
            {
                instr = node_instr node_type data_type instr_offset | instr_constant node instr_offset symtab,
                rd = if node_has_output node instr_offset then register instr_no else 0,
                rs1 = node_get_instr_arg node_index node registers 0 instr_no instr_offset,
                rs2 = node_get_instr_arg node_index node registers 1 instr_no instr_offset
            }
        )

let compile_node [tree_size] [max_vars] (tree: Tree[tree_size]) (symtab: Symtab[max_vars]) (registers: []i64) (instr_offset: [tree_size]i64)
        (node_index: i64) =
    let node = tree.nodes[node_index] in
    let node_instr = instr_offset[node_index] in
        [
            get_node_instr node node_instr node_index registers symtab 0,
            if has_instr node.node_type 1 then get_node_instr node (node_instr+1) node_index registers symtab 1 else (-1, -1, EMPTY_INSTR)
        ]

let check_idx_node_depth [tree_size] (tree: Tree[tree_size]) (depth: u32) (i: i64) =
    is_level tree.nodes[i] depth

let bit_width (x: u32): i32 = u32.num_bits - (u32.clz x)

let compile_tree [tree_size] [max_vars] (tree: Tree[tree_size]) (symtab: Symtab[max_vars]) (instr_offset: [tree_size]i64) (max_instrs: i64) =
    let idx_array = iota tree_size |> radix_sort (bit_width tree.max_depth) (\bit idx -> u32.get_bit bit tree.nodes[idx].depth)
    let depth_starts = iota tree_size |> filter (\i -> i == 0 || tree.nodes[idx_array[i]].depth != tree.nodes[idx_array[i-1]].depth)
    let initial_registers = replicate (tree_size * PARENT_IDX_PER_NODE) 0i64
    let initial_instr = replicate max_instrs EMPTY_INSTR in
    let (instr_result, _) =
        loop (data, registers) = (initial_instr, initial_registers) for i < i64.u32 tree.max_depth+1 do
            let j = (i64.u32 tree.max_depth)-i
            let start = depth_starts[j]
            let end = if j == i64.u32 tree.max_depth then tree_size else depth_starts[j + 1]
            let (idx, parent_idx, instrs) =
                idx_array[start:end] |>
                map (compile_node tree symtab (copy registers) instr_offset) |>
                flatten |>
                unzip3
            in
            (
                scatter data idx instrs,
                scatter registers parent_idx (map (\x -> x.rd) instrs)
            )
    in
        instr_result