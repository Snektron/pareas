module data_type = {
    type t = u8

    let invalid: t = 0
    let void: t = 1
    let int: t = 2
    let float: t = 3
    let int_ref: t = 4
    let float_ref: t = 5

    let is_ref (dty: t): bool =
        dty == int_ref || dty == float_ref

    let add_ref (dty: t): t =
        if dty == int then int_ref
        else if dty == float then float_ref
        else invalid

    let remove_ref (dty: t): t =
        if dty == int_ref then int
        else if dty == float_ref then float
        else invalid

    let is_comparable (dty: t): bool =
        dty == int || dty == float

    let is_castable (from: t) (to: t): bool =
        (from == int || from == float) && (to == int || to == float)

    let is_arithmetic (dty: t): bool =
        dty == int || dty == float
}

type data_type = data_type.t
