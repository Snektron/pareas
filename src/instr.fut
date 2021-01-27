import "tree"
import "datatypes"

let node_instr (node_type: NodeType, data_type: DataType) : u32 =
    match (node_type, data_type)
    --Binary integer arithmetic
    case (#add_expr, #int) ->       0b0000000_00000_00000_000_00000_0110011 --ADD
    case (#sub_expr, #int) ->       0b0100000_00000_00000_000_00000_0110011 --SUB
    case (#mul_expr, #int) ->       0b0000001_00000_00000_000_00000_0110011 --MUL
    case (#div_expr, #int) ->       0b0000001_00000_00000_100_00000_0110011 --DIV
    case (#mod_expr, #int) ->       0b0000001_00000_00000_110_00000_0110011 --REM
    case (#bitand_expr, #int) ->    0b0000000_00000_00000_111_00000_0110011 --AND
    case (#bitor_expr, #int) ->     0b0000000_00000_00000_110_00000_0110011 --OR
    case (#bitxor_expr, #int) ->    0b0000000_00000_00000_100_00000_0110011 --XOR
    case (#lshift_expr, #int) ->    0b0000000_00000_00000_001_00000_0110011 --SLL
    case (#rshift_expr, #int) ->    0b0100000_00000_00000_101_00000_0110011 --SRA
    case (#urshift_expr, #int) ->   0b0000000_00000_00000_101_00000_0110011 --SRL
    --Unary integer arithmetic
    case (#bitnot_expr, #int) ->    0b1111111_11111_00000_100_00000_0010011 --XORI -1
    case (#lnot_expr, #int) ->      0b0000000_00001_00000_011_00000_0010011 --SLTUI 1
    --Assignment
    case (#assign_expr, #int) ->    0b0000000_00000_00000_010_00000_0100011 --SW offset 0
    --Invalid opcodes
    case _ ->                       0b0000000_00000_00000_000_00000_1110011 --EBREAK

let compile_node [tree_size] [out_buffer_size] (tree: Tree[tree_size]) (node_index: u32) (output_buffer: [out_buffer_size]u32) =
    0i32