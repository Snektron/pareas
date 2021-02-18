import "tree"
import "datatypes"

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

let node_instr_0(node_type: NodeType) (data_type: DataType) : u32 =
    match (node_type, data_type)
    --Language constructs without instruction
    case (#statement_list, _) ->    0b0000000_00000_00000_000_00000_0000000 -- ignored
    case (#empty_stat, _) ->        0b0000000_00000_00000_000_00000_0000000 -- ignored
    case (#expr_stat, _) ->         0b0000000_00000_00000_000_00000_0000000 -- ignored

    -- Scope control
    case (#func_decl, _) ->         0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Control flow
    case (#if_stat, _) ->           0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#if_else_stat, _) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#else_aux, _) ->          0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#while_stat, _) ->        0b0000000_00000_00000_000_00000_1110011 -- TODO


    -- Binary integer arithmetic
    case (#add_expr, #int) ->       0b0000000_00000_00000_000_00000_0110011 -- ADD
    case (#sub_expr, #int) ->       0b0100000_00000_00000_000_00000_0110011 -- SUB
    case (#mul_expr, #int) ->       0b0000001_00000_00000_000_00000_0110011 -- MUL
    case (#div_expr, #int) ->       0b0000001_00000_00000_100_00000_0110011 -- DIV
    case (#mod_expr, #int) ->       0b0000001_00000_00000_110_00000_0110011 -- REM
    case (#bitand_expr, #int) ->    0b0000000_00000_00000_111_00000_0110011 -- AND
    case (#bitor_expr, #int) ->     0b0000000_00000_00000_110_00000_0110011 -- OR
    case (#bitxor_expr, #int) ->    0b0000000_00000_00000_100_00000_0110011 -- XOR
    case (#lshift_expr, #int) ->    0b0000000_00000_00000_001_00000_0110011 -- SLL
    case (#rshift_expr, #int) ->    0b0100000_00000_00000_101_00000_0110011 -- SRA
    case (#urshift_expr, #int) ->   0b0000000_00000_00000_101_00000_0110011 -- SRL

    -- Binary float arithmetic
    case (#add_expr, #float) ->     0b0000000_00000_00000_111_00000_1010011 -- FADD.S
    case (#sub_expr, #float) ->     0b0000100_00000_00000_111_00000_1010011 -- FSUB.S
    case (#mul_expr, #float) ->     0b0001000_00000_00000_111_00000_1010011 -- FMUL.S
    case (#div_expr, #float) ->     0b0001100_00000_00000_111_00000_1010011 -- FDIV.S

    -- Logical nodes
    case (#land_expr, #int) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#lor_expr, #int) ->       0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Unary integer arithmetic
    case (#bitnot_expr, #int) ->    0b1111111_11111_00000_100_00000_0010011 -- XORI -1
    case (#lnot_expr, #int) ->      0b0000000_00001_00000_011_00000_0010011 -- SLTUI 1
    case (#neg_expr, #int) ->       0b0100000_00000_00000_000_00000_0110011 -- SUB ri, x0

    -- Unary float arithmetic
    case (#neg_expr, #float) ->     0b0010000_00000_00000_001_00000_1010011 -- FSGNJN.S (ensure same operand twice)

    -- Integer comparision
    case (#eq_expr, #int) ->        0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#neq_expr, #int) ->       0b0100000_00000_00000_000_00000_0110011 -- SUB
    case (#less_expr, #int) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#great_expr, #int) ->     0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#lesseq_expr, #int) ->    0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#greateq_expr, #int) ->   0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Float comparision: TODO, change this lookup, return type is int
    case (#eq_expr, #float) ->      0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#neq_expr, #float) ->     0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#less_expr, #float) ->    0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#great_expr, #float) ->   0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#lesseq_expr, #float) ->  0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#greateq_expr, #float) -> 0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Assignment
    case (#assign_expr, #int) ->    0b0000000_00000_00000_010_00000_0100011 -- SW offset 0

    -- Function call
    case (#func_call_expr, _) ->    0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#func_call_arg, _) ->     0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Literals
    case (#lit_expr, _) ->          0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Casts
    case (#cast_expr, _) ->         0b0000000_00000_00000_000_00000_1110011 -- TODO

    -- Variables
    case (#decl_expr, _) ->         0b0000000_00000_00000_000_00000_1110011 -- TODO
    case (#id_expr, _) ->           0b0000000_00000_00000_000_00000_1110011 -- TODO

    --Invalid opcodes
    case _ ->                       0b0000000_00000_00000_000_00000_1110011 -- EBREAK

let node_has_return_0(_ : NodeType) (data_type : DataType) : bool =
    data_type != #void

let node_get_parent_arg_idx_0 (node: Node) : i64 =
    match node.node_type
        case #lit_expr -> -1
        case #decl_expr -> -1
        case #id_expr -> -1
        case _ ->
            if node_has_return_0 node.node_type node.resulting_type then
                i64.u32 node.parent * PARENT_IDX_PER_NODE + i64.u32 node.child_idx
            else
                -1

let node_get_instr_arg_0 (node_id: i64) (node: Node) (registers: []i64) (arg_no: i64) =
    let child_idx = match(node.node_type, arg_no)
        case (#add_expr, _) -> arg_no
        case (#sub_expr, _) -> arg_no
        case (#mul_expr, _) -> arg_no
        case (#div_expr, _) -> arg_no
        case (#mod_expr, _) -> arg_no
        case (#bitand_expr, _) -> arg_no
        case (#bitor_expr, _) -> arg_no
        case (#bitxor_expr, _) -> arg_no
        case (#lshift_expr, _) -> arg_no
        case (#rshift_expr, _) -> arg_no
        case (#urshift_expr, _) -> arg_no
        --TODO, other node types
        case _ -> -1
    in
        if child_idx == -1 then
            0
        else
            registers[node_id * PARENT_IDX_PER_NODE + child_idx]

let register (instr_no: i64) =
    instr_no + 32

let get_node_instr_0(node: Node) (instr_no: i64) (node_index: i64) (registers: []i64): (i64, i64, Instr) =
    let node_type = node.node_type
    let data_type = node.resulting_type
    in
        (
            instr_no,
            node_get_parent_arg_idx_0 node,
            {
                instr = node_instr_0 node_type data_type,
                rd = if node_has_return_0 node_type data_type then register instr_no else 0,
                rs1 = node_get_instr_arg_0 node_index node registers 0,
                rs2 = node_get_instr_arg_0 node_index node registers 1
            }
        )

let compile_node [tree_size] (tree: Tree[tree_size]) (registers: []i64) (instr_offset: [tree_size]i64) (node_index: i64) =
    let node = tree.nodes[node_index] in
    let node_instr = instr_offset[node_index] in
        [
            get_node_instr_0 node node_instr node_index registers,
            (-1, -1, EMPTY_INSTR) --TODO, get second instr offset
        ]

let check_idx_node_depth [tree_size] (tree: Tree[tree_size]) (depth: u32) (i: i64) =
    is_level tree.nodes[i] depth

let compile_tree [tree_size] (tree: Tree[tree_size]) (instr_offset: [tree_size]i64) =
    let idx_array = 0..<tree_size
    let initial_registers = replicate (tree_size * PARENT_IDX_PER_NODE) 0i64
    let initial_instr = replicate tree_size EMPTY_INSTR in
    let (instr_result, _) =
        loop (data, registers) = (initial_instr, initial_registers) for i < i64.u32 tree.max_depth do
            let (idx, parent_idx, instrs) =
                filter (check_idx_node_depth tree (tree.max_depth-(u32.i64 i-1))) idx_array |>
                map (compile_node tree (copy registers) instr_offset) |>
                flatten |>
                unzip3
            in
            (
                scatter data idx instrs,
                scatter registers parent_idx (map (\x -> x.rd) instrs)
            )
    in
        instr_result