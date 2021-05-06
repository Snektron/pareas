import "instr"
import "register"

let can_remove (i: Instr) (register_usage: []bool) =
    if i.rd < 64 then
        false
    else
        !register_usage[i.rd - 64]

let has_side_effect (i: Instr) =
    let opcode = i.instr & 0b0000000_00000_00000_111_00000_1111111 in
    opcode ==  0b0000000_00000_00000_010_00000_0100011 || opcode == 0b0000000_00000_00000_010_00000_0100111

let optimize_unused [n] [m] (instr: [n]Instr, functab: [m]FuncInfo) =
    let initial_used_registers_length = 2 * n
    let initial_used_registers = instr |>
        map (\i -> (i.rs1 - 64, i.rs2 - 64)) |>
        unzip |> 
        \(a, b) -> flatten [a, b]
    let used_registers = scatter (replicate n false) (initial_used_registers :> [initial_used_registers_length]i64) (replicate initial_used_registers_length true)
    --let used_registers_i = copy used_registers
    let (used_registers, _) = loop (used_registers, continue) = (used_registers, true) while continue do
        let used_registers_cpy = copy used_registers
        let newly_used = instr |>
            map (\i -> if can_remove i used_registers_cpy then (i.rs1 - 64, i.rs2 - 64) else (-1, -1)) |>
            unzip |>
            \(a, b) -> flatten [a, b]

        let new_used_registers = scatter used_registers (newly_used :> [initial_used_registers_length]i64) (replicate initial_used_registers_length false)
        let side_effect_correct = instr |>
            map (\i -> if has_side_effect i then (i.rs1 - 64, i.rs2 - 64) else (-1, -1)) |>
            unzip |>
            \(a, b) -> flatten [a, b]

        let result = scatter new_used_registers (side_effect_correct :> [initial_used_registers_length]i64) (replicate initial_used_registers_length true)
        let continue = !(map2 (==) used_registers_cpy result |> reduce (&&) true)
        in
        (
            result,
            continue
        )
    let used_instrs = iota n |>
        map (\i -> instr[i].rd < 64 || used_registers[i])
    in
    (instr, functab, used_instrs)


let optimize [n] [m] (instr: [n]Instr) (functab: [m]FuncInfo) =
    (instr, functab) |> optimize_unused