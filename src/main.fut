module tree = import "tree"

let main (n : i64) : u32 =
    let t = tree.alloc_tree n
    let result = tree.walk_tree t
    in result