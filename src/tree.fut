
let TREE_SIZE : u32 = 10

type Node = {
    parent: u32,
    depth: u32,
    value_id: u32
}

let alloc_tree(tree_size: u32) =
    let default_node : Node = {parent = 0, depth = 0, value_id = 0} in {
        nodes = replicate (i64.u32 tree_size) default_node,
        values = replicate (i64.u32 tree_size) 0u32, --TODO: change value type
        tree_size = tree_size
    }