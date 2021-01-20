
let TREE_SIZE : u32 = 10

--Node definition
type Node = {
    parent: u32,
    depth: u32,
    value_id: u32
}

--Tree definition
type Tree [tree_size] = {
    nodes: [tree_size]Node, --Nodes of the tree
    values: [tree_size]u32, --Tree values
    max_depth: u32 --Tree depth
}

--Type definition for the values of the nodes
type NodeValue = u32 --Temporary type

--Allocates a tree of a given size
let alloc_tree (tree_size: i64) =
    let default_node : Node = {parent = 0, depth = 0, value_id = 0}
    let default_node_value : NodeValue = 0
    in {
        nodes = replicate (tree_size) default_node,
        values = replicate (tree_size) default_node_value,
        max_depth = 0u32
    }

--Walks the tree, reducing the tree to a single result
let walk_tree [tree_size] (tree: Tree[tree_size]) =
    tree.max_depth
