import "tree"
import "datatypes"

let node_instr_0(node_type: NodeType, data_type: DataType) : u32 =
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

type Instr = {
    instr: u32,
    rd: u32,
    rs1: u32,
    rs2: u32
}

let get_node_instr(node: Node, instr_no: u32) : Instr =
    let node_type = node.node_type
    let data_type = node.resulting_type
    in
        {
            instr = match instr_no
                        case 0 -> node_instr_0(node_type, data_type)
                        case _ ->   0,
            rd = 0,
            rs1 = 0,
            rs2 = 0
        }

let compile_node [tree_size] (tree: Tree[tree_size]) (node_index: i64) : [2]Instr =
    let node : Node = tree.nodes[node_index]
    in
        [get_node_instr(node, 0), get_node_instr(node, 1)]