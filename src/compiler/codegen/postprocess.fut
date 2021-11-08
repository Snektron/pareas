import "instr"

let copy_instr_with_jt (instr: Instr) (jt: u32) =
    {
        instr = instr.instr,
        rd = instr.rd,
        rs1 = instr.rs1,
        rs2 = instr.rs2,
        jt = jt
    }

let is_jump (instr: Instr) =
    let instr_class = instr.instr & 0x7F
    in
    instr_class == 0b1100111

let is_branch (instr: Instr) =
    let instr_class = instr.instr & 0x7F
    in
    instr_class == 0b1100011

let make_auipc (constant: u32) : Instr =
    let opcode = 0b00001_0010111 | (constant << 12)
    in
    {
        instr = opcode,
        rd = 0,
        rs1 = 0,
        rs2 = 0,
        jt = 0
    }

let make_jump (instr: Instr) (constant: u32) : Instr =
    let opcode = instr.instr | (constant << 20) | (0b00001 << 15)
    in
    {
        instr = opcode,
        rd = instr.rd,
        rs1 = instr.rs1,
        rs2 = instr.rs2,
        jt = 0
    }

let process_jump (instr_loc: i64) (instr: Instr) =
    let target = instr.jt * 4
    let upper_offset = (target & 0xFFFFF000) >> 12
    let lower_bits = target & 0xFFF
    let sign_bit = (target >> 11) & 0x1
    let upper_bit_constant = upper_offset + sign_bit
    in
    [
        (instr_loc, make_auipc upper_bit_constant),
        (instr_loc+1, make_jump instr lower_bits)
    ]

let make_branch (instr: Instr) (delta: u32) : Instr =
    let bit_12 = (delta >> 12) & 0x1
    let bit_11 = (delta >> 11)  & 0x1
    let bit_10_5 = (delta >> 5) & 0x3F
    let bit_1_4 = (delta >> 1) & 0xF

    let opcode = instr.instr |
        (bit_12 << 31) |
        (bit_11 << 7) |
        (bit_10_5 << 25) |
        (bit_1_4 << 8)

    in
    {
        instr = opcode,
        rd = instr.rd,
        rs1 = instr.rs1,
        rs2 = instr.rs2,
        jt = 0
    }

let process_branch (instr_loc: i64) (instr: Instr) =
    let target = i64.u32 instr.jt * 4
    let delta = target - instr_loc * 4
    in
    [
        (instr_loc, make_branch instr (u32.i64 delta)),
        (-1, EMPTY_INSTR)
    ]


let finalize_jumps [n] (instr : [n]Instr) =
    let instr_offsets = instr |> 
        map (\i -> if is_jump i then 2i64 else 1i64) |>
        scan (+) 0 |>
        rotate (-1)
    let num_instr = if n == 0 then 0 else instr_offsets[0]
    let instr_offsets = iota n |> map (\i -> if i == 0 then 0 else instr_offsets[i])
    let new_instr = scatter (replicate num_instr EMPTY_INSTR) instr_offsets instr |>
        map (\i -> copy_instr_with_jt i (u32.i64 instr_offsets[i64.u32 i.jt]))

    let (jump_offsets, jump_instr) = iota n |> map (\i ->
        let instr = new_instr[instr_offsets[i]]
        in
        if is_jump instr then
            process_jump instr_offsets[i] instr
        -- else if is_branch instr then
        --     process_branch instr_offsets[i] instr
        else
            [(-1, EMPTY_INSTR), (-1, EMPTY_INSTR)]
        ) |>
        flatten |>
        unzip2

    let new_instr = scatter (copy new_instr) jump_offsets jump_instr
    in
    --iota n |> map (\i -> copy_instr_with_jt instr[i] (u32.i64 instr_offsets[i]))
    (new_instr, instr_offsets |> map i32.i64)

let finalize_instr_opcode (opcode: u32) (rd: i64) (rs1: i64) (rs2: i64) : u32 =
    let rd = rd & 0x1F
    let rs1 = rs1 & 0x1F
    let rs2 = rs2 & 0x1F

    in

    opcode |
        (u32.i64 rd << 7) |
        (u32.i64 rs1 << 15) |
        (u32.i64 rs2 << 20)

let finalize_instr [n] (instr: [n]Instr) =
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
