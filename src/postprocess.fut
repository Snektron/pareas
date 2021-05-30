import "instr"

let finalize_instr_opcode (opcode: u32) (rd: i64) (rs1: i64) (rs2: i64) : u32 =
    let rd = rd & 0x1F
    let rs1 = rs1 & 0x1F
    let rs2 = rs2 & 0x1F

    in

    opcode |
        (u32.i64 rd << 7) |
        (u32.i64 rs1 << 15) |
        (u32.i64 rs2 << 20)

let finalize_instr [n] (instr: [n]Instr) : [n]Instr =
    instr |> 
        map (\i ->
            {
                instr = finalize_instr_opcode i.instr i.rd i.rs1 i.rs2,
                rd = i.rd,
                rs1 = i.rs1,
                rs2 = i.rs2,
                jt = i.jt
            }
        )