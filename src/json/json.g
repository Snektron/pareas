json -> value;

value [string] -> 'string' maybe_member;
value [number] -> 'number';
value [true] -> 'true';
value [false] -> 'false';
value [nul] -> 'nul';
value [object] -> 'lbrace' maybe_values 'rbrace';
value [array] -> 'lbracket' maybe_values 'rbracket';

maybe_values [values] -> value value_list;
maybe_values [no_values]-> ;

value_list -> 'comma' value value_list;
value_list [value_list_end] -> ;

maybe_member [member] -> 'colon' value;
maybe_member [no_member] -> ;
