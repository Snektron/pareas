import "instr"

type FuncInfo = {
    id: u32,
    start: u32,
    size: u32
}

let INVALID_SYMBOL = 0xFFu8
let NUM_SYSTEM_REGS = 64i64

let is_valid_register (symbol_registers: []u8) (register: i64) =
    if register < NUM_SYSTEM_REGS then
        false
    else
        let symb_reg = symbol_registers[register-NUM_SYSTEM_REGS] in
        symb_reg != INVALID_SYMBOL

let deallocate_register (symbol_registers: []u8) (register: i64) (lifetime_mask: u64) =
    if register == 0 then
        lifetime_mask
    else if register < NUM_SYSTEM_REGS then
        lifetime_mask & !(1u64 << u64.i64 register)
    else
        let symb_reg = symbol_registers[register-NUM_SYSTEM_REGS] in
        if symb_reg != INVALID_SYMBOL then
            lifetime_mask & !(1u64 << u64.u8 symb_reg)
        else
            lifetime_mask

let allocate_register (symbol_registers: []u8) (register: i64) (lifetime_mask: u64) =
    if register == 0 then
        (lifetime_mask, INVALID_SYMBOL)
    else if register < NUM_SYSTEM_REGS then
        (lifetime_mask | (1u64 << u64.i64 register), u8.i64 register)
    else
        let free_reg = u8.i32 (u64.ctz (!lifetime_mask)) --TODO: check if no registers available
        in
        (lifetime_mask | (1u64 << u64.u8 free_reg), free_reg)

let lifetime_analyze_valid [n] (instrs: [n]Instr) (symbol_registers: []u8) (instr_offset: u32) (lifetime_mask: u64) (delta: u32) =
    let instr = instrs[i64.u32 instr_offset]
    let new_lifetime_mask_base = lifetime_mask |> deallocate_register symbol_registers instr.rs1 |> deallocate_register symbol_registers instr.rs2
    let (new_lifetime_mask, result_register) = new_lifetime_mask_base |> allocate_register symbol_registers instr.rd
    let register_info = [
        (if is_valid_register symbol_registers instr.rd then instr.rd else -1, result_register),
        (if is_valid_register symbol_registers instr.rs1 then instr.rs1 else -1, INVALID_SYMBOL),
        (if is_valid_register symbol_registers instr.rs2 then instr.rs2 else -1, INVALID_SYMBOL)
    ]
    in
    (0u32, new_lifetime_mask, 0u32, register_info)

let lifetime_analyze [n] (instrs: [n]Instr) (symbol_registers: []u8) (instr_offset: u32) (lifetime_mask: u64) (delta: u32) =
    let register_info = [(-1i64, INVALID_SYMBOL), (-1i64, INVALID_SYMBOL), (-1i64, INVALID_SYMBOL)] in
    if instr_offset == 0xFFFFFFFF then
        (0u32, lifetime_mask, delta, register_info)
    else
        lifetime_analyze_valid instrs symbol_registers instr_offset lifetime_mask delta

let current_func_offset(i: i64) (f: FuncInfo) =
    if i >= i64.u32 f.size then
        0xFFFFFFFFu32
    else
        u32.i64 i + f.start 

let register_alloc [n] [m] (instrs: [n]Instr) (functions: [m]FuncInfo) =
    let max_func_size = functions |> map (.size) |> u32.maximum |> i64.u32
    let instr_offsets_init = replicate n 0u32
    let lifetime_masks_init = replicate m 0x0000000000000001u64
    let delta_init = replicate m 0u32
    let symbol_registers_init = replicate n INVALID_SYMBOL

    let results = loop (instr_offsets, lifetime_masks, delta, symbol_registers) = (instr_offsets_init, lifetime_masks_init, delta_init, symbol_registers_init) for i < max_func_size do
        let old_offsets = map (current_func_offset i) functions 
        let (new_offsets, lifetime_masks, new_delta, updated_symbols) = map3 (lifetime_analyze instrs symbol_registers) old_offsets lifetime_masks delta |> unzip4
        let (symbol_offsets, symbol_data) = updated_symbols |> flatten |> unzip2
        in
        (
            scatter instr_offsets (old_offsets |> map i64.u32) new_offsets,
            lifetime_masks,
            new_delta,
            scatter symbol_registers symbol_offsets symbol_data
        )

    in

    (results.0, results.1, results.2)