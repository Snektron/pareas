import "string_packing"

module type strtab = {
    type element
    type offset
    type terminal

    val table_size: i64
    val table: [table_size]element
    val initial: (offset, offset)
    val get: (a: terminal) -> (b: terminal) -> (offset, offset)
}

module parser (stack_change_table: strtab) (partial_parse_table: strtab) = {

}
