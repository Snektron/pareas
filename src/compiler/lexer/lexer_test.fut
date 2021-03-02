module transition = u16
let transition_produces_token_mask: transition.t = 0x8000
let result_state_index (t: transition.t): u16 = t & !transition_produces_token_mask
