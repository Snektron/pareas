import "datatypes"

type Variable = {
    decl_type: DataType,
    global: bool,
    offset: u32
}

type Symtab [symtab_var_size] = {
    variables: [symtab_var_size]Variable
}