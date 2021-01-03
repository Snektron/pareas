
let TREE_SIZE : u32 = 10

type Node = {
    parent: u32,
    depth: u32,
    value_id: u32
}

type Tree [tree_size] = {
    nodes: [tree_size]Node,
    values: [tree_size]u32,
    max_depth: u32
}

let alloc_tree (tree_size: u32) =
    let default_node : Node = {parent = 0, depth = 0, value_id = 0}
    in {
        nodes = replicate (i64.u32 tree_size) default_node,
        values = replicate (i64.u32 tree_size) 0u32, --TODO: change value type
        max_depth = 0u32
    }

let walk_tree (tree_size: i64) (tree: Tree[tree_size]) =
    tree.max_depth