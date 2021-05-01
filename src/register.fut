import "instr"

type FuncInfo = {
    id: u32,
    start: u32,
    size: u32
}

type SymbolData = {
    register: u8,
    active: bool,
    swapped: bool
}

let EMPTY_SYMBOL_DATA : SymbolData = {
    register = 0u8,
    active = false,
    swapped = false
}

let INVALID_SYMBOL = 0xFFu8
let NUM_SYSTEM_REGS = 64i64

let is_valid_register (symbol_registers: []SymbolData) (register: i64) =
    if register < NUM_SYSTEM_REGS then
        false
    else
        let symb_reg = symbol_registers[register-NUM_SYSTEM_REGS].register in
        symb_reg != INVALID_SYMBOL

let needs_float_register (instr: u32) (offset: u32) =
    instr & 0b1111111 == 0b1010011

let find_free_register (instr: u32) (lifetime_mask: u64) (offset: u32) : u64 =
    let fixed_registers = lifetime_mask | (0xFFFFFFFFu64 << (if needs_float_register instr offset then 0 else 32))
    in
    u64.ctz (!fixed_registers) |> u64.i32

let lifetime_analyze_valid [n] (instrs: [n]Instr) (symbol_registers: []SymbolData) (instr_offset: u32) (lifetime_mask: u64) =
    if instr_offset == 0xFFFFFFFF then
        (0i32, lifetime_mask, [(-1, EMPTY_SYMBOL_DATA), (-1, EMPTY_SYMBOL_DATA), (-1, EMPTY_SYMBOL_DATA)])
    else
        let instr = instrs[i64.u32 instr_offset]
        let rd_register = find_free_register instr.instr lifetime_mask 0
        let new_lifetime_mask = lifetime_mask | (1u64 << rd_register)
        let register_info = [
            (if is_valid_register symbol_registers instr.rd then instr.rd else -1, EMPTY_SYMBOL_DATA),
            (if is_valid_register symbol_registers instr.rs1 then instr.rs1 else -1, EMPTY_SYMBOL_DATA),
            (if is_valid_register symbol_registers instr.rs2 then instr.rs2 else -1, EMPTY_SYMBOL_DATA)
        ]
        in
        (0i32, new_lifetime_mask, register_info)

let lifetime_analyze [n] (instrs: [n]Instr) (symbol_registers: []SymbolData) (instr_offset: u32) (lifetime_mask: u64) =
    let register_info = [(-1i64, EMPTY_SYMBOL_DATA), (-1i64, EMPTY_SYMBOL_DATA), (-1i64, EMPTY_SYMBOL_DATA)] in
    if instr_offset == 0xFFFFFFFF then
        (0i32, lifetime_mask, register_info)
    else
        lifetime_analyze_valid instrs symbol_registers instr_offset lifetime_mask

let current_func_offset(i: i64) (f: FuncInfo) =
    if i >= i64.u32 f.size then
        0xFFFFFFFFu32
    else
        u32.i64 i + f.start 

let register_alloc [n] [m] (instrs: [n]Instr) (functions: [m]FuncInfo) =
    let max_func_size = functions |> map (.size) |> u32.maximum |> i64.u32
    let instr_offsets_init = replicate n 0i32
    let lifetime_masks_init = replicate m 0x0000000000000001u64
    let symbol_registers_init = replicate n EMPTY_SYMBOL_DATA

    let results = loop (instr_offsets, lifetime_masks, symbol_registers) = (instr_offsets_init, lifetime_masks_init, symbol_registers_init) for i < max_func_size do
        let old_offsets = map (current_func_offset i) functions 
        let (new_offsets, lifetime_masks, updated_symbols) = map2 (lifetime_analyze instrs symbol_registers) old_offsets lifetime_masks |> unzip3
        let (symbol_offsets, symbol_data) = updated_symbols |> flatten |> unzip2
        in
        (
            scatter instr_offsets (old_offsets |> map i64.u32) new_offsets,
            lifetime_masks,
            scatter symbol_registers symbol_offsets symbol_data
        )

    in

    (results.0, results.1)