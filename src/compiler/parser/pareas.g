start -> program;

program -> fn_decl program;
program [program_end] -> ;

fn_decl -> 'fn' compound_stat;

stat [stat_while] -> 'while' expr compound_stat;
stat [stat_if] -> 'if' expr compound_stat;
stat [stat_else] -> 'else' compound_stat; # LL(P) doesn't support else statements otherwise
stat [stat_expr] -> expr 'semi';
stat [stat_compound] -> compound_stat;

compound_stat -> 'lbrace' stat_list 'rbrace';

stat_list -> stat stat_list;
stat_list [stat_list_end] -> ;

expr -> sum;

sum -> prod sum_list;

sum_list [sum_add] -> 'plus' prod sum_list;
sum_list [sum_sub] -> 'binary_minus' prod sum_list;
sum_list [sum_end] -> ;

prod -> unary prod_list;

prod_list [prod_mul] -> 'star' unary prod_list;
prod_list [prod_div] -> 'slash' unary prod_list;
prod_list [prod_end] -> ;

unary [unary_neg] -> 'unary_minus' unary;
unary [unary_bitflip] -> 'tilde' unary;
unary [unary_atom] -> atom;

atom [atom_paren] -> 'lparen' expr 'rparen';
atom [atom_variable] -> 'id';
