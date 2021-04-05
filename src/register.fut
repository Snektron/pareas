import "instr"

type FuncInfo = {
    id: u32,
    start: u32,
    size: u32
}

let lifetime_analyze [n] (instrs: [n]Instr) (instr_offset: u32) (lifetime_mask: u64) (delta: u32) =
    (0u32, lifetime_mask, 0u32)


let register_alloc [n] [m] (instrs: [n]Instr) (functions: [m]FuncInfo) =
    let max_func_size = functions |> map (.size) |> reduce u32.max 0 |> i64.u32
    let instr_offsets_init = replicate n 0u32
    let lifetime_masks_init = replicate m 0u64
    let delta_init = replicate m 0u32
    in

    loop (instr_offsets, lifetime_masks, delta) = (instr_offsets_init, lifetime_masks_init, delta_init) for i < max_func_size do
        let old_offsets = map (\j -> u32.i64 i + j.start) functions 
        let (new_offsets, lifetime_masks, new_delta) = map3 (lifetime_analyze instrs) old_offsets lifetime_masks delta |> unzip3
        in
        (
            scatter instr_offsets (old_offsets |> map i64.u32) new_offsets,
            lifetime_masks,
            new_delta
        )