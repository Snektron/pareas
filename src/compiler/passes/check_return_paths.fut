import "util"
import "../util"
import "../datatypes"
import "../../../gen/pareas_grammar"

-- | The types of operations a node in a binary expression tree can have:
-- Either `&&` or `||`, or this node can produce a constant of either value.
local type operator = #and | #or | #value

-- | The type of a node in the binary expression tree: We have a value, an operator, a left and a right child.
local type bool_exp_node = (bool, operator, i32, i32)

-- | Evolve a binary tree to compute more results. This function has to be applied only log2 n times to
-- compute the entire tree.
local let iter [n] (expr: [n]bool_exp_node): [n]bool_exp_node =
    -- Some helper functions to get values from certain nodes.
    let is_value_known ptr = expr[ptr].1 == #value
    let value ptr = expr[ptr].0
    -- The actual iteration operator, which is applied to each node.
    let f (v: bool, op: operator, left: i32, right: i32) =
        -- If this node already has a concrete value, then do nothing, as we are done with this node.
        if op == #value then
            (v, op, left, right)
        -- If either child is invalid, we don't need to look at the operator at all, and we can either pass up
        -- the value or provide a "leaf" value.
        else if left == -1 || right == -1 then
            -- If this node has only one child, then simply pass up the value of that.
            if left != -1 then expr[left]
            else if right != -1 then expr[right]
            -- Corner case: This node has no children (and it is not stat_return, else op would be #value),
            -- so just return false.
            else (false, #value, left, right)
        -- After this, we know that left and right are both valid (non-negative) pointers.
        -- If both children are known, we can simply compute the value.
        else if is_value_known left && is_value_known right then
            if op == #and then (value left && value right, #value, left, right)
            else (value left || value right, #value, left, right)
        -- If only one of the values is known, we can either produce an early result or pass up the value,
        -- depending on the operator and the known value.
        else if is_value_known left then
            -- If the value of the child is `true` and the operator is #and, then this operator can still become `false`,
            -- so we need to pass up the other child.
            -- Conversely, if the value of the child is `false` and the operator is #or, then this operator can still become
            -- `true` and we need to pass up the other child as well.
            if value left == (op == #and) then expr[right]
            -- Else, the value is equal to that of the known child.
            else expr[left]
        else if is_value_known right then
            -- Same thing as the previous branch but the sides swapped
            if value right == (op == #and) then expr[left]
            else expr[right]
        else
            (v, op, left, right)
    in map f expr

-- | As type resolving works by first computing a type that is valid for the expression if the program is valid,
-- we cannot also check whether all paths in a function return a value, as this would require picking the child with the
-- right value when analysing an `if_else` node. Instead, this check is performed separately in this pass.
let check_return_paths [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data_types: [n]data_type.t) =
    -- The children of each node are going to be its first child and its sibling, so compute those.
    let next_siblings = invert prev_siblings
    let first_childs =
        prev_siblings
        |> map (== -1)
        |> map2 (\parent first_child -> if first_child then parent else -1) parents
        |> invert
    -- We will also need to know whether a function returns void, and we will need to know this from the fn_decl node,
    -- so scatter up the void-ness.
    -- TODO: Should probably just compute this type before, as it's also used for `check_return_types`@term@"type_resolution".
    let is_void_fn_decl =
        let is =
            map2
                (\nty dty -> nty == production_atom_fn_proto && dty == data_type.void)
                node_types
                data_types
            |> map2 (\parent void_fn_decl -> if void_fn_decl then parent else -1) parents
            |> map i64.i32
        in scatter
            (replicate n false)
            is
            (replicate n true)
        -- Build the boolean expression tree.
        -- First, produce the initial value and operator.
    in map3
        -- Map nodes which pass up their value to #or so we can simply replace -1 pointers with -1 as value.
        -- If the operator is not #value, then the value doesn't matter, so we simply set it to false.
        (\nty parent next_sibling ->
            if nty == production_stat_return then
                (true, #value)
            else if parent == -1 then
                (false, #or)
            else if node_types[parent] == production_stat_if_else then
                -- Second child becomes and-type node.
                if nty == production_stat_list && next_sibling != -1 then (false, #and)
                else (false, #or)
            else if node_types[parent] == production_stat_list then
                (false, #or)
            else if node_types[parent] == production_stat_if || node_types[parent] == production_stat_while then
                -- Cannot guarantee these types returning, so return false from these
                (false, #value)
            else
                (false, #or))
        node_types
        parents
        next_siblings
    -- Now add the children
    |> map3
        (\first_child next_sibling (v: bool, op: operator) -> (v, op, first_child, next_sibling) : bool_exp_node)
        first_childs
        next_siblings
    -- Apply the computation function log2(n) times.
    |> iterate
        (n |> i32.i64 |> bit_width)
        iter
    -- Fetch the result value. At this point we know that all operators must be #value.
    |> map (.0)
    -- All function declaration nodes need to have 'true' unless they return void.
    |> map2 (||) is_void_fn_decl
    |> map2
        (||)
        (map (!= production_fn_decl) node_types)
    -- And finally this must hold for all nodes.
    |> reduce (&&) true
