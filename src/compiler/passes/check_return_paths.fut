import "util"
import "../util"
import "../datatypes"
import "../../../gen/pareas_grammar"

local type value = #true | #false | #unknown

local type operator = #and | #or | #true | #false

local type bool_exp_node = (value, operator, i32, i32)

local let iter [n] (expr: [n]bool_exp_node): [n]bool_exp_node =
    let get_node_value ptr =
        if ptr == -1 then #false
        else expr[ptr].0
    in map
        (\(v: value, op, left, right) ->
            let l = get_node_value left
            let r = get_node_value right
            in if v != #unknown then
                (v, op, left, right)
            else if op == #true then
                (#true, op, left, right)
            else if op == #false then
                (#false, op, left, right)
            -- No children are known: postpone computation
            else if l == #unknown && r == #unknown then
                (v, op, left, right)
            else if op == #and then
                if l == #false || r == #false then
                    (#false, op, left, right)
                else if l == #true && r == #true then
                    (#true, op, left, right)
                -- Partial applications: Import value from child
                else if (left == -1 || l == #true) && right != -1 then
                    expr[right]
                else if (right == -1 || r == #true) && left != -1 then
                    expr[left]
                -- Corner case: This node has no children, just return false
                else
                    (#false, op, left, right)
            else -- if op == #or then
                if l == #true || r == #true then
                    (#true, op, left, right)
                else if l == #false && r == #false then
                    (#false, op, left, right)
                -- Partial applications: Import value from child
                else if (left == -1 || l == #false) && right != -1 then
                    expr[right]
                else if (right == -1 || r == #false) && left != -1 then
                    expr[left]
                -- Corner case: This node has no children, just return false
                else
                    (#false, op, left, right))
        expr

let check_return_paths [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (data_types: [n]data_type.t) =
    let next_siblings = invert prev_siblings
    -- First, build the boolean expression tree:
    let expr_types: [n]operator =
        -- First, get the bool expr node type for each node.
        map3
            -- Map nodes which pass up their value to #or so we can simply replace -1 pointers with -1 as value.
            (\nty parent next_sibling ->
                if nty == production_stat_return then #true
                else if parent == -1 then #or
                else if node_types[parent] == production_stat_if_else then
                    -- Second child becomes and-type node.
                    if nty == production_stat_list && next_sibling != -1 then #and
                    else #or
                else if node_types[parent] == production_stat_list then
                    if next_sibling == -1 then #or
                    else #or
                else if node_types[parent] == production_stat_if || node_types[parent] == production_stat_while then
                    -- Cannot guarantee these types returning, so return false from these
                    #false
                else #or)
            node_types
            parents
            next_siblings
    let first_child =
        prev_siblings
        |> map (== -1)
        |> map2 (\parent first_child -> if first_child then parent else -1) parents
        |> invert
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
    in zip4
        (replicate n (#unknown : value))
        expr_types
        first_child
        next_siblings
    |> iterate
        (n |> i32.i64 |> bit_width)
        iter
    |> map (.0)
    |> map (== #true)
    -- All function declaration nodes need to have 'true' unless they return void.
    |> map2 (||) is_void_fn_decl
    |> map2
        (||)
        (map (!= production_fn_decl) node_types)
    |> reduce (&&) true
