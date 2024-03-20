import "util"
import "../util"
import "../datatypes"
import "../../../gen/pareas_grammar"

-- | The types of operations a node in a binary expression tree can have:
-- Either `&&` or `||`, or this node can produce a constant of either value.
local type bool_expr_node_type = #false | #true | #and | #or

-- | The type of a node in the binary expression tree: We have a value, an operator, a left and a right child.
local type bool_expr_node = (i32, i32, bool_expr_node_type)

-- | Evolve a binary tree to compute more results. This function has to be applied only log2 n times to
-- compute the entire tree.
local let iter [n] (expr: [n]bool_expr_node): [n]bool_expr_node =
    let is_value_known ptr = expr[ptr].2 == #true || expr[ptr].2 == #false
    let value ptr = expr[ptr].2 == #true
    let f (left, right, nty : bool_expr_node_type) =
        -- If this node already has a concrete value, then do nothing, as we are done with this node.
        if nty == #true || nty == #false then
            (left, right, nty)
        -- If either child is invalid, we don't need to look at the operator at all. We can either pass up
        -- the value or provide a "leaf" value.
        else if left == -1 || right == -1 then
            -- If this node has only one child, then simply pass up the value of that.
            if left != -1 then expr[left]
            else if right != -1 then expr[right]
            -- Corner case: This node has no children, so just return false.
            else (left, right, #false)
        -- After this, we know that left and right are both valid (non-negative) pointers.
        -- If both children are known, we can simply compute the value.
        else if is_value_known left && is_value_known right then
            if nty == #and then (left, right, if value left && value right then #true else #false)
            else (left, right, if value left || value right then #true else #false)
        else if is_value_known left then
            -- If the value of the child is `true` and the operator is #and, then this operator can still become `false`,
            -- so we need to pass up the other child.
            -- Conversely, if the value of the child is `false` and the operator is #or, then this operator can still become
            -- `true` and we need to pass up the other child as well.
            if value left == (nty == #and) then expr[right]
            -- Else, the value is equal to that of the known child.
            else expr[left]
        else if is_value_known right then
            -- Same thing as the previous branch but the sides swapped
            if value right == (nty == #and) then expr[left]
            else expr[right]
        -- Neither child is known: postpone computation.
        else
            (left, right, nty)
    in map f expr

-- | As type resolving works by first computing a type that is valid for the expression if the program is valid,
-- we cannot also check whether all paths in a function return a value, as this would require picking the child with the
-- right value when analysing an `if_else` node. Instead, this check is performed separately in this pass.
let check_return_paths [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32): bool =
    -- The children of each node are going to be its first child and its sibling, so compute those.
    let next_siblings = invert prev_siblings
    let first_childs =
        prev_siblings
        |> map (== -1)
        |> map2 (\parent first_child -> if first_child then parent else -1) parents
        |> invert
    -- Build the boolean expression tree.
    -- First, produce the initial value and operator.
    in map3
        -- Nodes which have only one child/which pass up their value are simply mapped to #or.
        (\nty parent next_sibling ->
            if nty == production_stat_return then #true
            else if parent == -1 then #or
            -- Only the second child of an if/else node becomes and-type node.
            else if node_types[parent] == production_stat_if_else && nty == production_stat_list && next_sibling != -1 then #and
            -- Cannot guarantee these types returning, so return false from these.
            else if node_types[parent] == production_stat_if || node_types[parent] == production_stat_while then #false
            else if nty == production_fn_decl then #and
            -- The return type of a void function maps to true as these don't need to end every path with a return statement.
            else if nty == production_type_void && node_types[parent] == production_fn_decl then #true
            else #or)
        node_types
        parents
        next_siblings
    -- Now add the children
    |> zip3
        first_childs
        next_siblings
    -- Apply the computation function log2(n) times.
    |> iterate
        (n |> i32.i64 |> bit_width)
        iter
    -- At this point, the first node (the fn_decl_list, which is the first node since the last compactify stage)
    -- holds whether the program is correct.
    |> (.[0])
    |> (.2)
    |> (== (#true : bool_expr_node_type))
