module tree = import "tree"

let main(n : u32) =
    let tree = tree.alloc_tree n
    let result = tree.walk_tree n tree
    in result