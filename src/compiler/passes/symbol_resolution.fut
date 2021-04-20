import "util"
import "../util"
import "../../../gen/pareas_grammar"

-- | Utility function to merge two resolution vectors. The values in these should be mutually
-- exclusively larger than zero.
let merge_resolutions [n] (a: [n]i32) (b: [n]i32) =
    map2 i32.max a b

-- | This function resolves function calls and checks declarations:
-- - Function declarations are all on global scope (the parser makes sure of this), but should all
--   have a unique name.
-- - Function declarations may appear in any order relative to calls.
-- - Function calls should of course only call functions that exist.
-- This function checks all of the above, and assigns new information to the `data` array as follows:
-- - Each `fn_decl` node gains a unique, sequentual (0-based) ID
-- - The name ID of each `fn_call` node is replaced with the function ID of the called function.
-- A bool specifying whether the program is valid according to above constraints and the new data
-- array is returned.
let resolve_fns [n] (node_types: [n]production.t) (parents: [n]i32) (data: [n]u32): (bool, [n]i32) =
    -- The program is only valid if all functions are unique, and so we must check whether all
    -- elements in the data array corresponding with atom_fn_proto are unique. There are multiple
    -- ways to do this:
    -- - Extract all function name IDs (the value in data) and then perform a radix sort and checking
    --   adjacent values for uniqueness.
    -- - Use reduce_by_index over the function name IDs to count the amounts of functions with the samen ame
    --   and check if they are all <= 1. This requires more auxillary data, but should be a little
    --   faster when there are many functions that have a different name (which is assumed to be the
    --   general case).
    --   Furthermore, this same technique can be used to finally link the function calls to the function definitions,
    --   and as any filtering on function nodes requires us to make this array anyway, it seems justified.
    -- TODO: Maybe merge fn_id_by_name and the computation of all_unique?
    -- TODO: Maybe merge `fn_decl` and `atom_fn_proto` in some stage before this?
    let is_fn_proto = map (== production_atom_fn_proto) node_types
    let is_fn_call = map (== production_atom_fn_call) node_types
    -- Build an index vector which scatters a node to a location corresponding to its name ID.
    let is =
        data
        |> map i32.u32
        |> map2 (\is_fn_proto name_id -> if is_fn_proto then name_id else -1) is_fn_proto
    -- To check if they're all unique, just count the occurances.
    let all_unique =
        reduce_by_index
            -- n is quite a large upper bound here (we know the maximum function name ID
            -- cannot be larger than the amount of nodes, as each unique name is given an ID sequentially
            -- from 0 (see `link_names`@term@"tokenize").
            -- We could replace this with the amount of unique IDs in the program (which would
            -- need to be passed from `build_data_vector`@term@"tokenize"), or we could simply compute the
            -- largest ID here.
            -- Note: At this point, the tree is smaller than the original source, however, no nodes carrying
            -- semantic information (such as IDs) have been removed, so this is still a valid upper bound.
            (replicate n 0i32)
            (+)
            0
            (map i64.i32 is)
            (replicate n 1i32)
        |> all (<= 1)
    -- Build a vector which, for each name, points to the function that declares it.
    let fn_decl_by_name =
        -- First, build a vector which points to the proto that declares it.
        invert is
        -- And get their parents.
        |> map (\i -> if i == -1 then -1 else parents[i])
    -- Now do the actual resolution by, for each `fn_call` node, simply looking in the fn_decl_by_name array.
    let resolution =
        data
        |> map i32.u32
        |> map2 (\is_call name_id -> if is_call then copy fn_decl_by_name[name_id] else -1) is_fn_call
    -- These must all yield something other than -1.
    let calls_valid =
        resolution
        |> map (!= -1)
        |> map2 (==) is_fn_call
        |> reduce (&&) true
    in (all_unique && calls_valid, resolution)

-- | This function resolves variable declarations and reads.
let resolve_vars [n] (node_types: [n]production.t) (parents: [n]i32) (prev_siblings: [n]i32) (right_leafs: [n]i32) (data: [n]u32): (bool, [n]i32) =
    -- This helper function returns the next node in the declaration search order
    let search_order_next ty parent prev_sibling =
        let is_first_child = prev_sibling == -1
        in if parent == -1 then -1
        else if node_types[parent] == production_stat_list then
            -- The first child of a statement list should just point up the tree, to its parent
            if is_first_child then parent
            -- Now, we want to either point to the previous sibling or the right leaf of the previous sibling
            -- (if we also need to search that). The latter is only the case for statement expressions,
            -- but we also add it for return expressions. Even though these are invalid, there are no checks for them
            -- as of yet and so `return a: int = 1; a = 2;` might otherwise produce unexpected errors.
            -- TODO: Maybe implement a check whether return is the last in a statement list.
            else if node_types[prev_sibling] == production_stat_expr || node_types[prev_sibling] == production_stat_return then right_leafs[prev_sibling]
            -- If we don't need to search through the subtree of that previous sibling, just point to the root of it.
            else prev_sibling
        else if node_types[parent] == production_stat_if || node_types[parent] == production_stat_if_else || node_types[parent] == production_stat_while then
            -- For `<keyword> condition block...;` type statements, we want don't want declarations in the condition or children
            -- to be visible from a node after the statement, but we do want declarations in the condition to be visible in the
            -- children. We also don't want declarations in child A to be visible in child B, so if the parent of a node is
            -- such a block, we want to search the condition expression, and should put our pointer to that.
            -- For if_else, we can just point to the root of the if part, which in turn points to the right leaf of the
            -- condition.
            --
            -- The first child should also go up to the parent
            if is_first_child then parent
            -- If this node is the second child, search the first child (the condition).
            else if prev_siblings[prev_sibling] == -1 then right_leafs[prev_sibling]
            else prev_sibling
        -- We don't want to continue searching after we reach a function declaration,
        -- as global variables are not supported for now.
        else if ty == production_fn_decl then -1
        -- These nodes are also not participating in variable lookup, but they shoulnd't ever be reached anyway as lookup
        -- stops at fn_decl. Set the pointer to end here just for good measure.
        else if ty == production_fn_decl_list then -1
        -- Otherwise, in expressions, we just want to search in reversed pre-order.
        else if is_first_child then parent
        else right_leafs[prev_sibling]
    -- Build the declaration search order vector.
    let search_order =
        -- Start with the complete search order over all nodes.
        map3
            search_order_next
            node_types
            parents
            prev_siblings
        -- We don't want to consider nodes which are not declarations, so filter them out here.
        -- These can be quite many, so we should use the logarithmic implementation here.
        -- This nicely reduces the number of iterations we need to do in the next step.
        |> flip
            find_unmarked_parents_log
            (map (!= production_atom_decl) node_types)
    -- Helper function to find the declaration for a particular variable name, starting at `start`.
    -- Returns the node index of the declaration, or -1 if there was none that matched the name id.
    let find_decl start name_id =
        loop current = start while current != -1 && data[current] != name_id do
            search_order[current]
    -- Now to do the actual lookup: For each variable read (atom_name) we're just going to iterate linearly over this list
    -- and attempt to find the accompanying declaration.
    let is_name_atom = map (== production_atom_name) node_types
    let resolution =
        map3
            (\start is_name name_id -> if is_name then find_decl start name_id else -1)
            search_order
            is_name_atom
            data
    -- Check if the source is valid simply by checking whether all names have a declaration associated with them.
    let valid =
        resolution
        |> map (!= -1)
        |> map2 (==) is_name_atom
        |> reduce (&&) true
    -- Finally, build the new data vector by replacing the name_id of `atom_decl` and `atom_name` with their offsets.
    in (valid, resolution)
