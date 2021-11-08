import "datatypes"

type Variable = {
    decl_type: DataType,
    offset: u32
}

type Symtab [symtab_var_size] = {
    variables: [symtab_var_size]Variable
}

let symtab_local_offset [var_size] (symtab: Symtab[var_size]) (var_id: u32) =
    symtab.variables[i64.u32 var_id].offset
