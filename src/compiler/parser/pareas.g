start -> fn_decl_list;

fn_decl_list -> fn_decl fn_decl_list;
fn_decl_list [fn_decl_list_end] -> ;

fn_decl -> 'fn' expr compound_stat;

type [type_int] -> 'int';
type [type_float] -> 'float';
type [type_void] -> 'void';

## Statements
stat [stat_while] -> 'while' expr compound_stat;
stat [stat_if] -> 'if' expr compound_stat;
stat [stat_else] -> 'else' compound_stat; # LL(P) doesn't support else statements otherwise
stat [stat_elif] -> 'elif' expr compound_stat;
stat [stat_expr] -> expr 'semi';
stat [stat_return] -> 'return' expr 'semi';
stat [stat_compound] -> compound_stat;

# Extra production type that can be used in the fix_if_else stage.
stat_if_else -> ;

compound_stat -> 'lbrace' stat_list 'rbrace';

stat_list -> stat stat_list;
stat_list [stat_list_end] -> ;

## Expressions
expr -> logical_or assign;

assign -> 'eq' logical_or assign;
assign [assign_end] -> ;

logical_or -> logical_and logical_or_list;

logical_or_list -> 'pipe_pipe' logical_and logical_or_list;
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

prod -> atom prod_list;

prod_list [prod_mul] -> 'star' atom prod_list;
prod_list [prod_div] -> 'slash' atom prod_list;
prod_list [prod_mod] -> 'percent' atom prod_list;
prod_list [prod_end] -> ;

atom [atom_unary_neg] -> 'unary_minus' atom;
atom [atom_unary_bitflip] -> 'tilde' atom;
atom [atom_unary_not] -> 'exclaim' atom;
atom [atom_paren] -> 'lparen' logical_or 'rparen';
atom [atom_id] -> 'id' maybe_app maybe_bind;
atom [atom_int] -> 'int_literal';
atom [atom_float] -> 'float_literal';

# Some extra nodes useful during the frontend part of the compilation.
atom_fn_call -> ; # Replaces atom_id if it has an application
atom_fn_proto -> ; # Replaces atom_fn_call if it has a bind
atom_decl -> ; # Replaces atom_id if it has a bind (but no call).

maybe_app [app] -> 'lbracket' args 'rbracket';
maybe_app [no_app] -> ;

maybe_bind [bind] -> 'colon' type;
maybe_bind [no_bind] -> ;

args -> expr arg_list;
args [no_args] -> ;

arg_list -> 'comma' expr arg_list;
arg_list [arg_list_end] -> ;
