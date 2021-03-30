start -> fn_decl_list;

fn_decl_list -> fn_decl fn_decl_list;
fn_decl_list [fn_decl_list_end] -> ;

fn_decl -> 'fn' compound_stat;

# Statements
stat [stat_while] -> 'while' expr compound_stat;
stat [stat_if] -> 'if' expr compound_stat;
stat [stat_else] -> 'else' compound_stat; # LL(P) doesn't support else statements otherwise
stat [stat_expr] -> expr 'semi';
stat [stat_compound] -> compound_stat;

compound_stat -> 'lbrace' stat_list 'rbrace';

stat_list -> stat stat_list;
stat_list [stat_list_end] -> ;

# Expressions
expr -> logical_or;

logical_or -> rela logical_or_list;

logical_or_list -> 'pipe_pipe' rela logical_or_list;
logical_or_list [logical_or_end] -> ;

logical_and -> rela logical_and_list;

logical_and_list -> 'and_and' rela logical_and_list;
logical_and_list [logical_and_end] -> ;

rela -> bitwise rela_list;

rela_list [rela_eq] -> 'eq_eq' bitwise rela_list;
rela_list [rela_neq] -> 'neq' bitwise rela_list;
rela_list [rela_gt] -> 'gt' bitwise rela_list;
rela_list [rela_gte] -> 'gte' bitwise rela_list;
rela_list [rela_lt] -> 'lt' bitwise rela_list;
rela_list [rela_lte] -> 'lte' bitwise rela_list;
rela_list [rela_end] -> ;

bitwise -> shift bitwise_list;

bitwise_list [bitwise_and] -> 'and' shift bitwise_list;
bitwise_list [bitwise_or] -> 'pipe' shift bitwise_list;
bitwise_list [bitwise_xor] -> 'hat' shift bitwise_list;
bitwise_list [bitwise_end] -> ;

shift -> sum shift_list;

shift_list [shift_lr] -> 'gt_gt' sum shift_list;
shift_list [shift_ar] -> 'gt_gt_gt' sum shift_list;
shift_list [shift_ll] -> 'lt_lt' sum shift_list;
shift_list [shift_end] -> ;

sum -> prod sum_list;

sum_list [sum_add] -> 'plus' prod sum_list;
sum_list [sum_sub] -> 'binary_minus' prod sum_list;
sum_list [sum_end] -> ;

prod -> unary prod_list;

prod_list [prod_mul] -> 'star' unary prod_list;
prod_list [prod_div] -> 'slash' unary prod_list;
prod_list [prod_mod] -> 'percent' unary prod_list;
prod_list [prod_end] -> ;

unary [unary_neg] -> 'unary_minus' unary;
unary [unary_bitflip] -> 'tilde' unary;
unary [unary_not] -> 'exclam' unary;
unary [unary_atom] -> atom;

atom [atom_paren] -> 'lparen' logical_or 'rparen';
atom [atom_variable] -> 'id';
