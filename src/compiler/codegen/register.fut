import "instr"
import "../../../lib/github.com/diku-dk/segmented/segmented"

type FuncInfo = {
    id: u32,
    start: u32,
    size: u32
}

type SymbolData = {
    register: u8,
    swapped: bool
}

let EMPTY_SYMBOL_DATA : SymbolData = {
    register = 0u8,
    swapped = false
}

let NUM_SYSTEM_REGS = 64i64
let PRESERVED_REGISTER_MASK = 0x0FFC0300_0FFC031Fu64
let NONSCRATCH_REGISTERS = 0x0FFC0300_0FFC0200u64

let is_system_register (register: i64) =
    register < NUM_SYSTEM_REGS

let needs_float_register (instr: u32) (offset: u32) =
    if instr == 0b1100000_00000_00000_111_00000_1010011 then -- Float -> int
        offset != 0
    else if instr == 0b1101000_00000_00000_111_00000_1010011 then -- Int -> float
        offset == 0
    else
        instr & 0b1111111 == 0b1010011

let find_free_register (float_reg: bool) (lifetime_mask: u64) =
    let fixed_registers = lifetime_mask | (0xFFFFFFFFu64 << (if float_reg then 0 else 32))
    in
    u64.ctz (!fixed_registers)

let make_symbol_data (reg: i32) =
    {
        register = u8.i32 reg,
        swapped = false
    }

let deallocate_register (d: SymbolData) =
    {
        register = d.register,
        swapped = d.swapped
    }

let get_symbol_data (symbol_registers: []SymbolData) (reg: i64) : SymbolData =
    if is_system_register reg then
        {
            register = u8.i64 reg,
            swapped = false
        }
    else
        symbol_registers[reg - NUM_SYSTEM_REGS]

let set_swap (data: SymbolData) : SymbolData = 
    {
        register = data.register,
        swapped = true
    }

let clear_register (reg: i32) (lifetime_mask: u64) =
    if reg == 0 then
        lifetime_mask
    else
        lifetime_mask & !(1 << u64.i32 reg)

let lifetime_analyze_is_call (instr: Instr) (function: FuncInfo) =
    let func_start = function.start
    let func_end = function.start + function.size
    in
    instr.instr == 0b0000000_00000_00000_000_00001_1101111 && instr.jt < func_start || instr.jt >= func_end

let lifetime_analyze_valid [n] (instrs: [n]Instr) (symbol_registers: []SymbolData) (instr_offset: u32) (lifetime_mask: u64) (register_state: [64]i64) (function: FuncInfo) =
    let instr = instrs[i64.u32 instr_offset]
    in
    if lifetime_analyze_is_call instr function then
        let register_info = [(-1i64, EMPTY_SYMBOL_DATA), (-1i64, EMPTY_SYMBOL_DATA), (-1i64, EMPTY_SYMBOL_DATA)]

        let new_lifetime_mask = lifetime_mask & PRESERVED_REGISTER_MASK
        let spilled_register_mask = lifetime_mask & !PRESERVED_REGISTER_MASK
        let spilled_registers = iota 64i64 |> map (\i -> 
                if spilled_register_mask & (1 << u64.i64 i) != 0 then
                    i
                else
                    -1
            )
        let new_register_state = iota 64i64 |> map (\i ->
                if new_lifetime_mask & (1 << u64.i64 i) != 0 then
                    register_state[i]
                else
                    -1
            )
        in
        --(new_lifetime_mask, register_info, spilled_registers, new_register_state)
        (new_lifetime_mask, register_info, spilled_registers, new_register_state)
    else
        let old_rs1_data = get_symbol_data symbol_registers instr.rs1
        let old_rs2_data = get_symbol_data symbol_registers instr.rs2
        -- let old_rd_data = get_symbol_data symbol_registers instr.rd

        let rs1_register = if old_rs1_data.swapped then if needs_float_register instr.instr 1 then 37 else 5 else i32.u8 old_rs1_data.register
        let rs2_register = if old_rs2_data.swapped then if needs_float_register instr.instr 2 then 38 else 6 else i32.u8 old_rs2_data.register
        let cleared_lifetime_mask = lifetime_mask |> clear_register rs1_register |> clear_register rs2_register

        let rd_register = if instr.rd < 64 then
                i32.i64 instr.rd
            else
                let float_reg = needs_float_register instr.instr 0
                let rd_tmp = find_free_register float_reg cleared_lifetime_mask
                in
                if rd_tmp == 64 then
                    if float_reg then 37 else 5 -- Use x5 for scratch
                else
                    rd_tmp
        let swap_rd = if rd_register != 0 && (cleared_lifetime_mask & (1u64 << u64.i32 rd_register) != 0) then rd_register else -1
        let swap_rs1 = if rs1_register != 0 && (lifetime_mask & (1u64 << u64.i32 rs1_register) != 0) then rs1_register else -1
        let swap_rs2 = if rs1_register != 0 && (lifetime_mask & (1u64 << u64.i32 rs2_register) != 0) then rs2_register else -1

        let new_lifetime_mask = cleared_lifetime_mask | (1u64 << u64.i32 rd_register)
        let register_info = [
            (if !(is_system_register instr.rd) then instr.rd - NUM_SYSTEM_REGS else -1, rd_register |> make_symbol_data),
            (if !(is_system_register instr.rs1) then instr.rs1 - NUM_SYSTEM_REGS else -1, old_rs1_data |> deallocate_register),
            (if !(is_system_register instr.rs2) then instr.rs2 - NUM_SYSTEM_REGS else -1, old_rs2_data |> deallocate_register)
        ]

        let (updated_register_no, updated_register_data) = [
            (i64.i32 rd_register, instr.rd),
            (if rs1_register == rd_register then -1 else i64.i32 rs1_register, -1),
            (if rs2_register == rd_register then -1 else i64.i32 rs2_register, -1)
        ] |> unzip2

        let new_register_state = scatter (copy register_state) updated_register_no updated_register_data
        let swapped_registers = [i64.i32 swap_rd, i64.i32 swap_rs1, i64.i32 swap_rs2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]
        in
        (new_lifetime_mask, register_info, swapped_registers, new_register_state)

let lifetime_analyze [n] (instrs: [n]Instr) (symbol_registers: []SymbolData) (enabled: [n]bool) (instr_offset: u32) (lifetime_mask: u64) (register_state: [64]i64) (function: FuncInfo)=
    let register_info = [(-1i64, EMPTY_SYMBOL_DATA), (-1i64, EMPTY_SYMBOL_DATA), (-1i64, EMPTY_SYMBOL_DATA)] in
    if instr_offset == 0xFFFFFFFF || !enabled[i64.u32 instr_offset] then
        (lifetime_mask, register_info, replicate 64 (-1i64), register_state)
    else
        lifetime_analyze_valid instrs symbol_registers instr_offset lifetime_mask register_state function

let current_func_offset(i: i64) (f: FuncInfo) =
    if i >= i64.u32 f.size then
        0xFFFFFFFFu32
    else
        u32.i64 i + f.start 

let count_instr [n] (instrs: [n]Instr) (symb_data: []SymbolData) (enabled: [n]bool) (instr: i64) : i32 =
    if enabled[instr] then
        1 -- Instruction itself
        + (if (get_symbol_data symb_data instrs[instr].rd).swapped then 1 else 0) -- Result register swap
        + (if (get_symbol_data symb_data instrs[instr].rs1).swapped then 1 else 0) --Instruction rs1 swap
        + (if (get_symbol_data symb_data instrs[instr].rs2).swapped then 1 else 0) --Instruction rs2 swap
    else
        0

let count_instr_add_preserve [n] [m] (functions: [m]FuncInfo) (preserve_masks: [m]u64) (instr_counts: [n]i32) =
    let (preserve_offsets, preserve_data) = iota m |>
        map (\i -> 
            let regs_preserved = u64.popc preserve_masks[i]
            let func_start_idx = i64.u32 (functions[i].start + 5)
            let func_end_idx = i64.u32 (functions[i].start + functions[i].size - 6)
            in
            [
                (func_start_idx, instr_counts[func_start_idx] + regs_preserved),
                (func_end_idx, instr_counts[func_end_idx] + regs_preserved)
            ]) |>
        flatten |>
        unzip2
    in
    scatter (copy instr_counts) preserve_offsets preserve_data

let regalloc_make_load (dest_reg: i64) (stack_offset: u32) : Instr =
    let instr_offset = (4*(stack_offset-1)) << 20
    in
    {
        instr =  0b0000000_00000_00000_010_00000_0000011 | instr_offset,
        rd = dest_reg,
        rs1 = 0,
        rs2 = 0,
        jt = 0
    }

let regalloc_make_store (src_reg: i64) (stack_offset: u32) : Instr =
    let stack_data = (-(4*(stack_offset-1)))
    let lower_bits = (stack_data & 0x1F) << 7
    let upper_bits = (stack_data & 0xFE) << 19
    in
    {
        instr =  0b0000000_00000_00000_010_00000_0100011 | lower_bits | upper_bits,
        rd = 0,
        rs1 = src_reg,
        rs2 = 0,
        jt = 0
    }

let regalloc_make_instr (instr: Instr) (rd_register: i64) (rs1_register: i64) (rs2_register: i64) (instr_offset: []i32) : Instr =
    {
        instr = instr.instr,
        rd = rd_register,
        rs1 = rs1_register,
        rs2 = rs2_register,
        jt = u32.i32 instr_offset[i64.u32 instr.jt]
    }

let register_alloc [n] [m] (instrs: [n]Instr, functions: [m]FuncInfo, enabled: [n]bool, stack_sizes: [m]u32)=
    let max_func_size = functions |> map (.size) |> u32.maximum |> i64.u32
    let lifetime_masks_init = replicate m 0b00000000_00000000_00000000_00000000_00000000_00000000_00000000_00011111u64
    let preserve_masks_init = replicate m 0u64
    let symbol_registers_init = replicate n EMPTY_SYMBOL_DATA
    let register_state = replicate m (replicate 64 (-1i64))

    let (_, preserve_masks, symbol_registers, _) = loop (lifetime_masks, preserve_masks, symbol_registers, register_state) = (lifetime_masks_init, preserve_masks_init, symbol_registers_init, register_state) for i < max_func_size do
        let old_offsets = map (current_func_offset i) functions
        let reg_state_copy = copy register_state
        let (lifetime_masks, updated_symbols, swapped_registers, register_state) = map4 (lifetime_analyze instrs symbol_registers enabled) old_offsets lifetime_masks register_state functions |> unzip4
        let swap_data =
            iota m |>
            map (
                \k ->
                swapped_registers[k] |>
                map (
                    \i ->
                        if i < 0 then
                            (-1, EMPTY_SYMBOL_DATA)
                        else
                            (reg_state_copy[k, i] - 64, get_symbol_data symbol_registers reg_state_copy[k, i] |> set_swap)
                    )
            ) |>
            flatten
        let symb_data = updated_symbols |> flatten
        let (symbol_offsets, symbol_data) = concat swap_data symb_data |> unzip2
        let preserve_masks = map2 (|) preserve_masks lifetime_masks
        in
        (
            lifetime_masks,
            preserve_masks,
            scatter symbol_registers symbol_offsets symbol_data,
            register_state
        )

    let preserve_masks = map (\i -> i & NONSCRATCH_REGISTERS) preserve_masks

    let func_start_bools = scatter (replicate n false) (functions |> map (.start) |> map i64.u32) (replicate m true)
    let reverse_func_id_map = func_start_bools |> map i64.bool |> scan (+) 0
    let spill_offsets = symbol_registers |>
                            map (.swapped) |>
                            map i64.bool |>
                            segmented_scan (+) 0 func_start_bools

    let instr_offsets = iota n |>
        map (count_instr instrs symbol_registers enabled) |>
        count_instr_add_preserve functions preserve_masks |>
        scan (+) 0 |>
        rotate (-1) |>
        map2 (\i x -> if i == 0 then 0 else x) (iota n)

    let new_instr_num = i64.i32 (if length instr_offsets == 0 then 0 else instr_offsets[(length instr_offsets)-1]+1)
    let new_instr = replicate new_instr_num EMPTY_INSTR

    let (new_instr_idx, new_instr_opcodes) = iota n |>
        map (\i ->
            if enabled[i] then
                let instr = instrs[i]
                let base_instr_offset = i64.i32 instr_offsets[i]
                let (rs1_load_offset, rs1_stack_offset) = if instr.rs1 >= 64 && symbol_registers[instr.rs1 - 64].swapped then (base_instr_offset, u32.i64 spill_offsets[instr.rs1-64]) else (-1, 0)
                let (rs2_load_offset, rs2_stack_offset) =
                    if instr.rs2 >= 64 && symbol_registers[instr.rs2 - 64].swapped then (
                            base_instr_offset +
                            if rs1_load_offset > 0 then 1 else 0
                            ,u32.i64 spill_offsets[instr.rs2-64]
                        )
                    else (-1, 0)
                let main_instr_offset = base_instr_offset + (if rs1_load_offset > 0 then 1 else 0) + (if rs2_load_offset > 0 then 1 else 0)
                let (rd_offset, rd_stack_offset) = if instr.rd >= 64 && symbol_registers[instr.rd - 64].swapped then (main_instr_offset + 1, u32.i64 spill_offsets[instr.rd-64]) else (-1,0)
                let func_id = reverse_func_id_map[i]-1

                let allocated_rd = if instr.rd < 64 then instr.rd else i64.u8 symbol_registers[instr.rd - 64].register
                let allocated_rs1 = if instr.rs1 < 64 then instr.rs1 else
                    let reg_data = symbol_registers[instr.rs1 - 64]
                    in
                    if reg_data.swapped then
                        if needs_float_register instr.instr 1 then
                            37
                        else
                            5
                    else
                        i64.u8 reg_data.register
                let allocated_rs2 = if instr.rs2 < 64 then instr.rs2 else
                    let reg_data = symbol_registers[instr.rs2 - 64]
                    in
                    if reg_data.swapped then
                        if needs_float_register instr.instr 2 then
                            38
                        else
                            6
                    else
                        i64.u8 reg_data.register

                in
                [
                    (rs1_load_offset, regalloc_make_load 5 (rs1_stack_offset + stack_sizes[func_id])), --Load spill to rs1
                    (rs2_load_offset, regalloc_make_load 6 (rs2_stack_offset + stack_sizes[func_id])), --Load spill to rs2
                    (main_instr_offset, regalloc_make_instr instr allocated_rd allocated_rs1 allocated_rs2 instr_offsets), --Main instruction
                    (rd_offset, regalloc_make_store allocated_rd (rd_stack_offset + stack_sizes[func_id])) --Store spill
                ]
            else
                [
                    (-1, EMPTY_INSTR),
                    (-1, EMPTY_INSTR),
                    (-1, EMPTY_INSTR),
                    (-1, EMPTY_INSTR)
                ]
        ) |>
        flatten |>
        unzip2

    let new_instr = scatter new_instr new_instr_idx new_instr_opcodes

    let overflows = replicate m 0u32

    in

    (
        instr_offsets,
        preserve_masks,
        symbol_registers |> map (\i -> i.register),
        overflows,
        symbol_registers |> map (\i -> i.swapped),
        new_instr
    )

let make_empty_instr (opcode: u32) : Instr =
    {
        instr = opcode,
        rd = 0,
        rs1 = 0,
        rs2 = 0,
        jt = 0
    }

let OPCODE_LUI_BP : u32 =  0b0000000_00000_00000_000_01000_0110111
let OPCODE_ADDI_BP : u32 = 0b0000000_00000_01000_000_01000_0010011
let OPCODE_STORE : u32 =   0b0000000_00000_01000_010_00000_0100011
let OPCODE_STOREF : u32 =  0b0000000_00000_01000_010_00000_0100111
let OPCODE_LOAD : u32 =    0b0000000_00000_01000_010_00000_0000011
let OPCODE_LOADF : u32 =   0b0000000_00000_01000_010_00000_0000111

let fill_stack_frames [n] [m] (functions: [m]FuncInfo) (func_symbols: [m]u32) (func_overflows: [m]u32) (instr: [n]Instr) (preserve_masks: [m]u64) =
    let scatter_info = iota m |>
        map (\i ->
            let preserve_count = u32.i32 (u64.popc preserve_masks[i])
            let stack_size = (func_symbols[i] + func_overflows[i] + preserve_count + 2) * 4
            let lower_bits = (stack_size & 0xFFF) << 20
            let upper_bits = stack_size - (signextend (stack_size & 0xFFF)) & 0xFFFFF000
            in
            [
                --(i64.u32 functions[i].start, make_empty_instr functions[i].size),
                (i64.u32 functions[i].start + 2, make_empty_instr (OPCODE_LUI_BP | upper_bits)),
                (i64.u32 functions[i].start + 3, make_empty_instr (OPCODE_ADDI_BP | lower_bits)),
                (i64.u32 functions[i].start + i64.u32 functions[i].size - 6, make_empty_instr (OPCODE_LUI_BP | upper_bits)),
                (i64.u32 functions[i].start + i64.u32 functions[i].size - 5, make_empty_instr (OPCODE_ADDI_BP | lower_bits))
            ]
        ) |>
        flatten

    let preserve_info = iota m |>
        map (\i ->
            let preserve_stack_offset = (func_symbols[i] + func_overflows[i] + 2) * 4
            let p_mask = preserve_masks[i]

            in
            iota 64 |>
                map (\j ->
                        let leading_regs = u64.popc (p_mask & ((1u64 << (u64.i64 j)) - 1u64))
                        let offset = -(preserve_stack_offset + (u32.i32 leading_regs) * 4)
                        let store_offset_high = (offset & 0xFE0u32) << 25
                        let store_offset_low = (offset & 0x1Fu32) << 7
                        let store_src = ((u32.i64 j) % 32) << 20
                        let store_const = store_offset_high | store_offset_low | store_src
                        let opcode = if j >= 32 then OPCODE_STOREF else OPCODE_STORE

                        in
                        if p_mask & (1 << u64.i64 j) > 0 then
                            (i64.u32 functions[i].start + i64.i32 leading_regs + 6, make_empty_instr (opcode | store_const))
                        else
                            (-1, EMPTY_INSTR)
                    )
            )
        |> flatten
    let load_info = iota m |>
        map (\i ->
            let preserve_stack_offset = (func_symbols[i] + func_overflows[i] + 2) * 4
            let p_mask = preserve_masks[i]

            in
            iota 64 |>
                map (\j ->
                        let leading_regs = u64.popc (p_mask & ((1u64 << (u64.i64 j)) - 1u64))
                        let offset = -(preserve_stack_offset + (u32.i32 leading_regs) * 4)

                        let load_offset = offset << 20
                        let load_dst = ((u32.i64 j) % 32) << 7
                        let load_const = load_offset | load_dst
                        let opcode = if j >= 32 then OPCODE_LOADF else OPCODE_LOAD

                        in
                        if p_mask & (1 << u64.i64 j) > 0 then
                            (i64.u32 functions[i].start + i64.u32 functions[i].size - i64.i32 leading_regs - 7, make_empty_instr (opcode | load_const))
                        else
                            (-1, EMPTY_INSTR)
                    )
            )
        |> flatten

    let all_info = concat (concat scatter_info preserve_info) load_info
    let (all_offsets, all_data) = all_info |> unzip
    in
    scatter (copy instr) all_offsets all_data
