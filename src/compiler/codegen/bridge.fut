import "tree"
import "datatypes"

type front_node_type = i32
type front_data_type = i32
type front_node_idx_type = i32
type front_depth_type = i32
type front_child_idx_type = i32
type front_node_data_type = u32

let NODE_TYPE_LOOKUP : []NodeType = [
    --TODO: fill in table to map node types
    0
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
    NODE_TYPE_LOOKUP[node_type]

let convert_data_type (data_type: front_data_type) =
    DATA_TYPE_LOOKUP[data_type]

let convert_node_idx (idx: front_node_idx_type) =
    idx

let convert_depth (depth: front_depth_type) =
    depth

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
        depth = convert_depth depth,
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

let backend_convert [n]
        (node_types: [n]front_node_type)
        (node_res_types: [n]front_data_type)
        (node_parents: [n]front_node_idx_type)
        (node_depth: [n]front_depth_type)
        (node_child_idx : [n]front_child_idx_type)
        (node_data: [n]front_node_data_type)
        (max_depth: front_depth_type)  : Tree[n] =
    
    let input = zip6 node_types node_res_types node_parents node_depth node_child_idx node_data

    let nodes: [n]Node = input |>
        map backend_convert_node

    in {
        nodes = nodes,
        max_depth = convert_depth max_depth
    }
