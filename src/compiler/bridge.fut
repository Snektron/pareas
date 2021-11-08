import "codegen/tree"
import "codegen/datatypes"
import "frontend"
import "datatypes"
import "passes/util"

module production = g.production
local open g

type front_node_type = production.t
type front_data_type = data_type
type front_node_idx_type = i32
type front_depth_type = i32
type front_child_idx_type = i32
type front_node_data_type = u32

-- Note: Keep in sync with pareas.g
let NODE_TYPE_LOOKUP = mk_production_array node_type_invalid [
        (production_fn_decl_list, node_type_statement_list),
        (production_fn_decl, node_type_func_decl),
        (production_fn_decl_dummy, node_type_func_decl_dummy),
        (production_stat_while, node_type_while_stat),
        (production_stat_if, node_type_if_stat),
        (production_stat_expr, node_type_expr_stat),
        (production_stat_return, node_type_return_stat),
        (production_while_dummy, node_type_while_dummy),
        (production_stat_if_else, node_type_if_else_stat),
        (production_stat_list, node_type_statement_list),
        (production_assign, node_type_assign_expr),
        (production_logical_or, node_type_lor_expr),
        (production_logical_and, node_type_land_expr),
        (production_rela_eq, node_type_eq_expr),
        (production_rela_neq, node_type_neq_expr),
        (production_rela_gt, node_type_great_expr),
        (production_rela_gte, node_type_greateq_expr),
        (production_rela_lt, node_type_less_expr),
        (production_rela_lte, node_type_lesseq_expr),
        (production_bitwise_and, node_type_bitand_expr),
        (production_bitwise_or, node_type_bitor_expr),
        (production_bitwise_xor, node_type_bitxor_expr),
        (production_shift_lr, node_type_rshift_expr),
        (production_shift_ar, node_type_urshift_expr),
        (production_shift_ll, node_type_lshift_expr),
        (production_sum_add, node_type_add_expr),
        (production_sum_sub, node_type_sub_expr),
        (production_prod_mul, node_type_mul_expr),
        (production_prod_div, node_type_div_expr),
        (production_prod_mod, node_type_mod_expr),
        (production_atom_unary_neg, node_type_neg_expr),
        (production_atom_unary_bitflip, node_type_bitnot_expr),
        (production_atom_unary_not, node_type_lnot_expr),
        (production_atom_cast, node_type_cast_expr),
        (production_atom_decl, node_type_decl_expr),
        (production_atom_name, node_type_id_expr),
        (production_atom_int, node_type_lit_expr),
        (production_atom_float, node_type_lit_expr),
        (production_atom_fn_call, node_type_func_call_expr),
        (production_atom_unary_deref, node_type_deref_expr),
        (production_atom_decl_explicit, node_type_decl_expr),
        (production_arg_list, node_type_func_call_arg_list),
        (production_arg, node_type_func_call_arg),
        (production_param_list, node_type_func_arg_list),
        (production_param, node_type_func_arg)
    ]

let DATA_TYPE_LOOKUP : []DataType = [
    0, --Invalid
    1, --Void
    2, --Int
    3, --Float
    4, --Int_ref
    5 --Float_ref
]

let convert_node_type (node_type: front_node_type) =
    NODE_TYPE_LOOKUP[i32.u8 node_type]

let convert_data_type (data_type: front_data_type) =
    DATA_TYPE_LOOKUP[i32.u8 data_type]

let convert_node_idx (idx: front_node_idx_type) =
    idx

let convert_child_idx (child_idx: front_child_idx_type) =
    child_idx

let convert_node_data (node_data: front_node_data_type) =
    node_data

let backend_convert_node (
        node_type: front_node_type,
        data_type: front_data_type,
        parent: front_node_idx_type,
        depth: front_depth_type,
        child_idx: front_child_idx_type,
        data: front_node_data_type) : Node =
    {
        node_type = convert_node_type node_type,
        resulting_type = convert_data_type data_type,
        parent = convert_node_idx parent,
        depth = depth,
        child_idx = convert_child_idx child_idx,
        node_data = convert_node_data data
    }

let zip6 [n] 'a 'b 'c 'd 'e 'f
    (x0: [n]a)
    (x1: [n]b)
    (x2: [n]c)
    (x3: [n]d)
    (x4: [n]e)
    (x5: [n]f) =

    let c1 = zip5 x0 x1 x2 x3 x4
    in
    map2 (\(t0, t1, t2, t3, t4) t5 -> (t0, t1, t2, t3, t4, t5)) c1 x5

let convert_ast [n]
        (node_types: [n]front_node_type)
        (node_res_types: [n]front_data_type)
        (node_parents: [n]front_node_idx_type)
        (node_depth: [n]front_depth_type)
        (node_child_idx : [n]front_child_idx_type)
        (node_data: [n]front_node_data_type): Tree[n] =
    let input = zip6 node_types node_res_types node_parents node_depth node_child_idx node_data
    let nodes: [n]Node = input |>
        map backend_convert_node
    in {
        nodes = nodes,
        max_depth = i32.maximum node_depth
    }
