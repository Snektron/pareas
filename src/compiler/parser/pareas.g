start -> fn_decl_list;

fn_decl_list -> fn_decl fn_decl_list;
fn_decl_list [fn_decl_list_end] -> ;

fn_decl -> 'fn' expr compound_stat;

type [type_int] -> 'int';
type [type_float] -> 'float';
type [type_void] -> 'void';

## Statements
stat [stat_while] -> 'while' while_dummy expr compound_stat;
stat [stat_if] -> 'if' expr compound_stat;
stat [stat_else] -> 'else' compound_stat; # LL(P) doesn't support else statements otherwise
stat [stat_elif] -> 'elif' expr compound_stat;
stat [stat_expr] -> maybe_expr 'semi';
stat [stat_return] -> 'return' maybe_expr 'semi';
stat [stat_compound] -> compound_stat;

while_dummy -> ;

# Extra production type that can be used in the fix_if_else stage.
stat_if_else -> ;

compound_stat -> 'lbrace' stat_list 'rbrace';

stat_list -> stat stat_list;
stat_list [stat_list_end] -> ;

## Expressions
# By making the LHS of this production the same as that of `expr`, we can ignore it further,
# and simply handle no_expr when type checking.
maybe_expr [expr_] -> logical_or_list assign;
maybe_expr [no_expr] -> ;

expr -> logical_or_list assign;

assign -> 'eq' logical_or_list assign;
assign [assign_end] -> ;

logical_or_list -> logical_and_list logical_or;

logical_or -> 'pipe_pipe' logical_and_list logical_or;
logical_or [logical_or_end] -> ;

logical_and_list -> rela_list logical_and;

logical_and -> 'and_and' rela_list logical_and;
logical_and [logical_and_end] -> ;

rela_list -> bitwise_list rela;

rela [rela_eq] -> 'eq_eq' bitwise_list rela;
rela [rela_neq] -> 'neq' bitwise_list rela;
rela [rela_gt] -> 'gt' bitwise_list rela;
rela [rela_gte] -> 'gte' bitwise_list rela;
rela [rela_lt] -> 'lt' bitwise_list rela;
rela [rela_lte] -> 'lte' bitwise_list rela;
rela [rela_end] -> ;

bitwise_list -> shift_list bitwise;

bitwise [bitwise_and] -> 'and' shift_list bitwise;
bitwise [bitwise_or] -> 'pipe' shift_list bitwise;
bitwise [bitwise_xor] -> 'hat' shift_list bitwise;
bitwise [bitwise_end] -> ;

shift_list -> sum_list shift;

shift [shift_lr] -> 'gt_gt' sum_list shift;
shift [shift_ar] -> 'gt_gt_gt' sum_list shift;
shift [shift_ll] -> 'lt_lt' sum_list shift;
shift [shift_end] -> ;

sum_list -> prod_list sum;

sum [sum_add] -> 'plus' prod_list sum;
sum [sum_sub] -> 'binary_minus' prod_list sum;
sum [sum_end] -> ;

prod_list -> atom prod;

prod [prod_mul] -> 'star' atom prod;
prod [prod_div] -> 'slash' atom prod;
prod [prod_mod] -> 'percent' atom prod;
prod [prod_end] -> ;

atom [atom_unary_neg] -> 'unary_minus' atom;
atom [atom_unary_bitflip] -> 'tilde' atom;
atom [atom_unary_not] -> 'exclaim' atom;
atom [atom_cast] -> type 'lparen' expr 'rparen';
atom [atom_paren] -> 'lparen' expr 'rparen';
atom [atom_name] -> 'name' maybe_app maybe_bind;
atom [atom_int] -> 'int_literal';
atom [atom_float] -> 'float_literal';

# Some extra nodes useful during the frontend part of the compilation.
atom_fn_call -> ; # Replaces atom_name if it has an application.
atom_fn_proto -> ; # Replaces atom_fn_call if it has a bind.
atom_decl -> ; # Replaces atom_name if it has a bind (but no call).
atom_unary_deref -> ; # Inserted when an l-value needs to be dereferenced.

maybe_app [app] -> 'lbracket' args 'rbracket';
maybe_app [no_app] -> ;

maybe_bind [bind] -> 'colon' type;
maybe_bind [no_bind] -> ;

args -> arg arg_list;
args [no_args] -> ;

arg_list -> 'comma' arg arg_list;
arg_list [arg_list_end] -> ;

arg -> expr; # Required for code generation.
