import "tree"
import "datatypes"
import "symtab"
import "../lib/github.com/diku-dk/sorts/radix_sort"

type Instr = {
    instr: u32,
    rd: i64,
    rs1: i64,
    rs2: i64,
    jt: u32
}

let EMPTY_INSTR : Instr = {
    instr = 0,
    rd = 0,
    rs1 = 0,
    rs2 = 0,
    jt = 0
}

let PARENT_IDX_PER_NODE : i64 = 3

let node_instr(node_type: NodeType) (data_type: DataType) (instr_offset: i64) : u32 =
    match (node_type, data_type, instr_offset)
    --Language constructs without instruction
    case (#statement_list, _, 0) ->     0b0000000_00000_00000_000_00000_0000000 -- ignored
    case (#empty_stat, _, 0) ->         0b0000000_00000_00000_000_00000_0000000 -- ignored
    case (#expr_stat, _, 0) ->          0b0000000_00000_00000_000_00000_0000000 -- ignored

    -- Scope control
    case (#func_decl_dummy, _, 0) ->        0b1111111_00001_00010_010_11100_1110011 -- SW -4(sp), ra
    case (#func_decl_dummy, _, 1) ->        0b1111111_01000_00010_010_11000_1110011 -- SW -8(sp), bp
    case (#func_decl_dummy, _, 2) ->        0b0100000_01000_00010_011_00010_0110011 -- SUB sp, sp, bp
    case (#func_decl_dummy, _, 3) ->        0b0000000_00010_01000_000_01000_0110011 -- ADD bp, bp, sp
    case (#func_decl, _, 0) ->              0b0000000_01000_00010_000_00010_0110011 -- ADD sp, sp, bp
    case (#func_decl, _, 1) ->              0b1111111_11100_00010_010_00001_0100011 -- LW ra, -4(sp)
    case (#func_decl, _, 2) ->              0b1111111_11000_00010_010_01000_0100011 -- LW bp, -8(sp)
    case (#func_decl, _, 3) ->              0b0000000_00000_00001_000_00000_1100111 -- JALR ra
    case (#func_arg, #int_ref, 0) ->        0b0000000_00000_00000_010_00000_0100011 -- SW offset 0
    case (#func_arg, #float_ref, 0) ->      0b0000000_00000_00000_010_00000_0100111 -- FSW offset 0
    case (#func_arg_float_in_int, _, 0) ->  0b0000000_00000_00000_010_00000_0100011 -- SW offset 0
    case (#func_arg_stack, _, _) ->         0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#func_arg_list, _, _) ->          0b0000000_00000_00000_000_00000_0000000 -- ignored

    -- Control flow
    case (#if_stat, _, 0) ->            0b0000000_00000_00000_000_00000_1100011 -- BEQ x0
    case (#if_else_stat, _, 0) ->       0b0000000_00000_00000_000_00000_1100011 -- BEQ x0
    case (#if_else_stat, _, 1) ->       0b0000000_00000_00000_000_00000_1101111 -- JAL x0
    case (#while_stat, _, 0) ->         0b0000000_00000_00000_000_00000_1100011 -- BEQ x0
    case (#while_stat, _, 1) ->         0b0000000_00000_00000_000_00000_1101111 -- JAL x0

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
    case (#eq_expr, #int, 0) ->         0b0100000_00000_00000_000_00000_0110011 -- SUB
    case (#eq_expr, #int, 1) ->         0b0000000_00001_00000_011_00000_0010011 -- SLTIU 1
    case (#neq_expr, #int, 0) ->        0b0100000_00000_00000_000_00000_0110011 -- SUB
    case (#neq_expr, #int, 1) ->        0b0000000_00000_00000_011_00000_0110011 -- SLTU x0, ri
    case (#less_expr, #int, 0) ->       0b0000000_00000_00000_010_00000_0110011 -- SLT
    case (#great_expr, #int, 0) ->      0b0000000_00000_00000_010_00000_0110011 -- SLT
    case (#lesseq_expr, #int, 0) ->     0b0000000_00000_00000_010_00000_0110011 -- SLT
    case (#lesseq_expr, #int, 1) ->     0b0000000_00001_00000_011_00000_0010011 -- SLTUI 1
    case (#greateq_expr, #int, 0) ->    0b0000000_00000_00000_010_00000_0110011 -- SLT
    case (#greateq_expr, #int, 1) ->    0b0000000_00001_00000_011_00000_0010011 -- SLTUI 1

    -- Float comparision
    case (#eq_expr, #float, 0) ->       0b1010000_00000_00000_010_00000_1010011 -- FEQ.S
    case (#neq_expr, #float, 0) ->      0b1010000_00000_00000_010_00000_1010011 -- FEQ.S
    case (#neq_expr, #float, 1) ->      0b0000000_00001_00000_011_00000_0010011 -- SLTUI 1
    case (#less_expr, #float, 0) ->     0b1010000_00000_00000_001_00000_1010011 -- FLT.S
    case (#great_expr, #float, 0) ->    0b1010000_00000_00000_001_00000_1010011 -- FLT.S
    case (#lesseq_expr, #float, 0) ->   0b1010000_00000_00000_000_00000_1010011 -- FLE.S
    case (#greateq_expr, #float, 0) ->  0b1010000_00000_00000_000_00000_1010011 -- FLE.S

    -- Assignment
    case (#assign_expr, #int, 0) ->     0b0000000_00000_00000_010_00000_0100011 -- SW offset 0
    case (#assign_expr, #int, 1) ->     0b0000000_00000_00000_000_00000_0110011 -- ADD x0
    case (#assign_expr, #float, 0) ->   0b0000000_00000_00000_010_00000_0100111 -- FSW offset 0
    case (#assign_expr, #float, 1) ->   0b0010000_00000_00000_000_00000_1010011 -- FSGNJ.S ry, ry

    -- Function call
    case (#func_call_expr, _, _) ->                 0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#func_call_arg, #int, 0) ->               0b0000000_00000_00000_000_00000_0110011 -- ADD x0
    case (#func_call_arg, #float, 0) ->             0b0010000_00000_00000_000_00000_1010011 -- FSGNJ.S ry, ry
    case (#func_call_arg_float_in_int, _, 0) ->     0b1110000_00000_00000_000_00000_1010011 -- FMV.X.W
    case (#func_call_arg_stack, _, _) ->            0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#func_call_arg_list, _, _) ->             0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Literals
    case (#lit_expr, #int, 0) ->        0b0000000_00000_00000_000_00000_0110111 -- LUI
    case (#lit_expr, #int, 1) ->        0b0000000_00000_00000_000_00000_0010011 -- ADDI
    case (#lit_expr, #float, 0) ->      0b0000000_00000_00000_000_00000_0110111 -- LUI
    case (#lit_expr, #float, 1) ->      0b0000000_00000_00000_000_00000_0010011 -- ADDI
    case (#lit_expr, #float, 2) ->      0b1111000_00000_00000_000_00000_1010011 -- FMV.W.X

    -- Casts (NOTE: filter out int->int or float->float casts in earlier stages, done with single map in preprocessing stage)
    case (#cast_expr, #int, 0) ->       0b1100000_00000_00000_111_00000_1010011 -- FCVT.W.S
    case (#cast_expr, #float, 0) ->     0b1101000_00000_00000_111_00000_1010011 -- FCVT.S.W
    case (#deref_expr, #int, 0) ->      0b0000000_00000_00000_010_00000_0000011 -- LW 0(rs1)
    case (#deref_expr, #float, 0) ->    0b0000000_00000_00000_010_00000_0000111 -- FLW 0(rs1)

    -- Variables
    case (#decl_expr, _, 0) ->          0b0000000_00000_01000_000_00000_0010011 -- ADDI bp (local offsets inverted)
    case (#id_expr, _, 0) ->            0b0000000_00000_01000_000_00000_0010011 -- ADDI bp (local offsets inverted)

    --Invalid opcodes
    case _ ->                           0b0000000_00000_00000_000_00000_1110011 -- EBREAK

let has_instr (node_type: NodeType) (data_type: DataType) (instr_offset: i64) : bool =
    match(node_type, data_type, instr_offset)
        case (#invalid, _, 0) -> false
        case (#statement_list, _, 0) -> false
        case (#empty_stat, _, 0) -> false
        case (#func_arg_list, _, 0) -> false
        case (#expr_stat, _, 0) -> false
        
        case (#func_decl_dummy, _, 1) -> true
        case (#func_decl_dummy, _, 2) -> true
        case (#func_decl_dummy, _, 3) -> true
        case (#func_decl, _, 1) -> true
        case (#func_decl, _, 2) -> true
        case (#func_decl, _, 3) -> true
        case (#if_else_stat, _, 1) -> true
        case (#while_stat, _, 1) -> true
        case (#eq_expr, #int, 1) -> true
        case (#neq_expr, _, 1) -> true
        case (#lesseq_expr, #int, 1) -> true
        case (#greateq_expr, #int, 1) -> true
        case (#lit_expr, _, 1) -> true
        case (#lit_expr, #float, 2) -> true
        case (#assign_expr, _, 1) -> true
        case (_, _, 0) -> true
        case _ -> false

let node_has_return(_ : NodeType) (data_type : DataType) : bool =
    !(data_type == #void || data_type == #invalid)

let parent_arg_idx (node: Node) : i64 =
    i64.i32 node.parent * PARENT_IDX_PER_NODE + i64.i32 node.child_idx

let node_get_parent_arg_idx_sub (node: Node) (instr_offset: i64) : i64 =
    match (node.node_type, node.resulting_type, instr_offset)
        case (#eq_expr, #int, 0) -> -1
        case (#neq_expr, _, 0) -> -1
        case (#lesseq_expr, #int, 0) -> -1
        case (#greateq_expr, #int, 0) -> -1
        case (#lit_expr, _, 0) -> -1
        case (#assign_expr, _, 0) -> -1
        case (_, _, 0) ->
            if node_has_return node.node_type node.resulting_type then
                parent_arg_idx node
            else
                -1

        case (#eq_expr, #int, 1) -> parent_arg_idx node
        case (#neq_expr, _, 1) -> parent_arg_idx node
        case (#lesseq_expr, #int, 1) -> parent_arg_idx node
        case (#greateq_expr, #int, 1) -> parent_arg_idx node
        case (#lit_expr, #int, 1) -> parent_arg_idx node
        case (#assign_expr, _, 1) -> parent_arg_idx node

        case (#lit_expr, #float, 2) -> parent_arg_idx node
        case _ -> -1

let node_get_parent_arg_idx (nodes: []Node) (node: Node) (instr_offset: i64) : i64 =
    if node.parent == INVALID_NODE_IDX then
        node_get_parent_arg_idx_sub node instr_offset
    else
        let parent = nodes[node.parent]
        in
        if node.child_idx > 0 && (parent.node_type == #if_stat || parent.node_type == #if_else_stat) then
            parent_arg_idx node
        else if (node.child_idx == 0 || node.child_idx == 2) && (parent.node_type == #while_stat) then
            parent_arg_idx node
        else
            node_get_parent_arg_idx_sub node instr_offset

let register (instr_no: i64) =
    instr_no + 64

let node_get_instr_arg (node_id: i64) (node: Node) (registers: []i64) (arg_no: i64) (instr_no: i64) (instr_offset: i64) : i64 =
    match(node.node_type, node.resulting_type, arg_no, instr_offset)
        case (#func_arg, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#func_arg, #int_ref, 1, 0) -> i64.u32 node.node_data + 10
        case (#func_arg, #float_ref, 1, 0) -> i64.u32 node.node_data + 42
        case (#func_arg_float_in_int, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#func_arg_float_in_int, _, 1, 0) -> i64.u32 node.node_data + 10
        case (#if_stat, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#if_else_stat, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#while_stat, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE + 1]
        case (#add_expr, _, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#sub_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#mul_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#div_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#mod_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#bitand_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#bitor_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#bitxor_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#lshift_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#rshift_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#urshift_expr, _, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#eq_expr, _, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#neq_expr, _, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#less_expr, _, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#great_expr, _, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + 1 - arg_no]
        case (#lesseq_expr, #int, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + 1 - arg_no]
        case (#lesseq_expr, #float, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#greateq_expr, #int, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#greateq_expr, #float, _, 0) -> registers[node_id * PARENT_IDX_PER_NODE + 1 - arg_no]
        case (#func_call_arg, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#func_call_arg, #float, 1, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#func_call_arg_float_in_int, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#assign_expr, _,_, 0) -> registers[node_id * PARENT_IDX_PER_NODE + arg_no]
        case (#cast_expr, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]
        case (#deref_expr, _, 0, 0) -> registers[node_id * PARENT_IDX_PER_NODE]

        case (#eq_expr, #int, 0, 1) -> register (instr_no - 1)
        case (#neq_expr, #int, 1, 1) -> register (instr_no - 1)
        case (#neq_expr, #float, 0, 1) -> register (instr_no - 1)
        case (#lesseq_expr, #int, 0, 1) -> register (instr_no - 1)
        case (#greateq_expr, #int, 0, 1) -> register (instr_no - 1)
        case (#assign_expr, _, 0, 1) -> registers[node_id * PARENT_IDX_PER_NODE + 1]
        case (#assign_expr, #float, 1, 1) -> registers[node_id * PARENT_IDX_PER_NODE + 1]
        case (#lit_expr, _, 0, 1) -> register (instr_no - 1)
        case (#lit_expr, #float, 0, 2) -> register (instr_no - 1)
        case _ -> 0

let node_has_output (nodes: []Node) (node: Node) (instr_offset: i64) : bool =
    (node_get_parent_arg_idx nodes node instr_offset) != -1 || match (node.node_type, node.resulting_type, instr_offset)
        case (#eq_expr, #int, 0) -> true
        case (#neq_expr, _, 0) -> true
        case (#lesseq_expr, #int, 0) -> true
        case (#greateq_expr, #int, 0) -> true
        case (#lit_expr, _, 0) -> true
        case (#lit_expr, #float, 1) -> true
        case _ -> false

-- let make_branch_constant (node_id: i64) (registers: []i64) (delta: i64) (instr_loc: i64) (idx: i64) : u32 =
--     let offset = (registers[node_id * PARENT_IDX_PER_NODE + idx] + delta - instr_loc) * 4
--     let low_offset = (((offset >> 1) & 0xF) << 1) | ((offset >> 11) & 0x1)
--     let high_offset = ((offset >> 5) & 0x1F) | (((offset >> 12) & 0x1) << 5)
--     in
--     u32.i64 ((low_offset << 7) | (high_offset << 25))

-- let make_jump_constant (node_id: i64) (registers: []i64) (instr_loc: i64) (idx: i64): u32 =
--     let offset = (registers[node_id * PARENT_IDX_PER_NODE + idx] - instr_loc) * 4
--     in
--     u32.i64 (
--         (((offset >> 12) & 0xFF) << 12) |
--         (((offset >> 11) & 0x1) << 20) |
--         (((offset >> 1) & 0x3FF) << 21) |
--         (((offset >> 20) & 0x1) << 31)
--     )

let signextend(x: u32) =
    let signed_x = i32.u32 x in
    u32.i32 (signed_x << 20 >> 20)
    
let instr_constant [max_vars] (node: Node) (instr_offset: i64) (symtab: Symtab[max_vars]) : u32 =
    match(node.node_type, node.resulting_type, instr_offset)
        case (#lit_expr, _, 0) -> node.node_data - (signextend (node.node_data & 0xFFF)) & 0xFFFFF000
        case (#lit_expr, _, 1) -> (node.node_data & 0xFFF) << 20
        case (#id_expr, _, 0) -> (-4 * ((symtab_local_offset symtab node.node_data) + 2)) << 20
        case (#decl_expr, _, 0) -> (-4 * ((symtab_local_offset symtab node.node_data) + 2)) << 20
        case _ -> 0

let instr_jt (node: Node) (node_id: i64) (instr_offset: i64) (registers: []i64) : i64 =
    match(node.node_type, instr_offset)
        case (#if_stat, 0) -> registers[node_id * PARENT_IDX_PER_NODE + 1]
        case (#if_else_stat, 0) -> registers[node_id * PARENT_IDX_PER_NODE + 1] + 1
        case (#if_else_stat, 1) -> registers[node_id * PARENT_IDX_PER_NODE + 2]
        case (#while_stat, 0) -> registers[node_id * PARENT_IDX_PER_NODE + 2] + 1
        case (#while_stat, 1) -> registers[node_id * PARENT_IDX_PER_NODE]
        case _ -> 0

let get_instr_loc (node: Node) (node_id: i64) (instr_no: i64) (instr_offset: i64) (registers: []i64) =
    if instr_offset == 1 && (node.node_type == #if_else_stat) then
        registers[node_id * PARENT_IDX_PER_NODE + 1]
    else if instr_offset == 1 && node.node_type == #while_stat then
        registers[node_id * PARENT_IDX_PER_NODE + 2]
    else if (instr_offset >= 2 && node.node_type == #func_decl_dummy) || node.node_type == #func_decl then
        instr_no + 2
    else
        instr_no

let get_data_prop_value [tree_size] (tree: Tree[tree_size]) (node: Node) (rd: i64) (instr_no: i64) =
    if node.parent == INVALID_NODE_IDX then
        rd
    else
        let parent_type = tree.nodes[node.parent].node_type in
        if parent_type == #if_stat || parent_type == #if_else_stat then
            if node.child_idx == 1 || node.child_idx == 2 then
                instr_no
            else
                rd
        else if parent_type == #while_stat then
            if node.child_idx == 0 || node.child_idx == 2 then
                instr_no
            else
                rd
        else
            rd

let get_output_register [tree_size] (tree: Tree[tree_size]) (node: Node) (instr_no: i64) (instr_offset: i64) =
    match (node.node_type, node.resulting_type)
        case (#func_call_arg, #int) -> i64.u32 node.node_data + 10
        case (#func_call_arg, #float) -> i64.u32 node.node_data + 42
        case (#func_call_arg_float_in_int, _) -> i64.u32 node.node_data + 10
        case _ -> if node_has_output tree.nodes node instr_offset then register instr_no else 0

let get_node_instr [tree_size] [max_vars] (tree: Tree[tree_size]) (node: Node) (instr_no: i64) (node_index: i64) (registers: []i64) (symtab: Symtab[max_vars]) (instr_offset: i64): (i64, i64, Instr, i64) =
    let node_type = node.node_type
    let data_type = node.resulting_type
    let rd = get_output_register tree node instr_no instr_offset
    let instr_loc = get_instr_loc node node_index instr_no instr_offset registers
    in
        (
            instr_loc,
            node_get_parent_arg_idx tree.nodes node instr_offset,
            {
                instr = node_instr node_type data_type instr_offset | instr_constant node instr_offset symtab,
                rd = rd,
                rs1 = node_get_instr_arg node_index node registers 0 instr_no instr_offset,
                rs2 = node_get_instr_arg node_index node registers 1 instr_no instr_offset,
                jt = u32.i64 (instr_jt node node_index instr_offset registers)
            },
            get_data_prop_value tree node rd instr_no
        )

let compile_node [tree_size] [max_vars] (tree: Tree[tree_size]) (symtab: Symtab[max_vars]) (registers: []i64) (instr_offset: [tree_size]i64)
        (node_index: i64) =
    let node = tree.nodes[node_index]
    let node_instr = instr_offset[node_index]
    in
        [
            if has_instr node.node_type node.resulting_type 0 then
                get_node_instr tree node node_instr node_index registers symtab 0
            else
                (-1, node_get_parent_arg_idx tree.nodes node 0, EMPTY_INSTR, get_data_prop_value tree node 0 node_instr)
            ,
            if has_instr node.node_type node.resulting_type 1 then get_node_instr tree node (node_instr+1) node_index registers symtab 1 else (-1, -1, EMPTY_INSTR, 0),
            if has_instr node.node_type node.resulting_type 2 then get_node_instr tree node (node_instr+2) node_index registers symtab 2 else (-1, -1, EMPTY_INSTR, 0),
            if has_instr node.node_type node.resulting_type 3 then get_node_instr tree node (node_instr+3) node_index registers symtab 3 else (-1, -1, EMPTY_INSTR, 0)
        ]

let check_idx_node_depth [tree_size] (tree: Tree[tree_size]) (depth: i32) (i: i64) =
    is_level tree.nodes[i] depth

let bit_width (x: i32): i32 =
    i32.num_bits - (i32.clz x)

let compile_tree [tree_size] [max_vars] (tree: Tree[tree_size]) (symtab: Symtab[max_vars]) (instr_offset: [tree_size]i64) (max_instrs: i64) =
    let idx_array = iota tree_size |> radix_sort (bit_width tree.max_depth) (\bit idx -> i32.get_bit bit tree.nodes[idx].depth)
    let depth_starts = iota tree_size |> filter (\i -> i == 0 || tree.nodes[idx_array[i]].depth != tree.nodes[idx_array[i-1]].depth)
    let initial_registers = replicate (tree_size * PARENT_IDX_PER_NODE) 0i64
    let initial_instr = replicate max_instrs EMPTY_INSTR
    let (instr_result, _) =
        loop (data, registers) = (initial_instr, initial_registers) for i < tree.max_depth+1 do
            let j = tree.max_depth-i
            let start = depth_starts[j]
            let end = if j == tree.max_depth then tree_size else depth_starts[j + 1]
            let (idx, parent_idx, instrs, new_regs) =
                idx_array[start:end] |>
                map (compile_node tree symtab (copy registers) instr_offset) |>
                flatten |>
                unzip4
            in
            (
                scatter data idx instrs,
                scatter registers parent_idx new_regs
            )
    in
        instr_result