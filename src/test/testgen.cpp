#include "test/coderenderer.hpp"
#include "test/renderstate.hpp"
#include "test/noderenderstate.hpp"
#include "test/textrenderstate.hpp"
#include "test/repeatrenderstate.hpp"
#include "test/optionrenderstate.hpp"
#include "test/idcreaterenderstate.hpp"
#include "test/indentrenderstate.hpp"
#include "test/scoperenderstate.hpp"
#include "test/randomintrenderstate.hpp"
#include "test/idrenderstate.hpp"
#include "test/commitrenderstate.hpp"

#include <memory>
#include <iostream>

using state_ptr = std::unique_ptr<RenderState>;

enum IdCategory {
    CATEGORY_DEFAULT = 0,
    CATEGORY_INT = 1
};

template <typename... T>
NodeRenderState* make_noderenderstate(const T&... args) {
    return new NodeRenderState({(args.get())...});
}

const RenderState* uniform_option_picker(CodeRenderer& renderer, const std::vector<const RenderState*>& states) {
    std::uniform_int_distribution<size_t> distr(0, states.size() - 1);

    return states[distr(renderer.get_rng())];
}

template <size_t Bias>
const RenderState* bias_first_option_picker(CodeRenderer& renderer, const std::vector<const RenderState*>& states) {
    std::uniform_int_distribution<size_t> distr(0, (Bias+1) * states.size());

    size_t val = distr(renderer.get_rng());
    if(val >= states.size()) {
        return states[0];
    }
    else {
        return states[val]; 
    }
}

template <size_t Bias>
const RenderState* bias_not_last_picker(CodeRenderer& renderer, const std::vector<const RenderState*>& states) {
    std::uniform_int_distribution<size_t> distr(0, Bias * (states.size()-1) + 1);

    size_t val = distr(renderer.get_rng());
    return states[val/Bias];
}

template <const RenderState*(*Base_op)(CodeRenderer&, const std::vector<const RenderState*>&)>
const RenderState* depth_based_option_picker(CodeRenderer& renderer, const std::vector<const RenderState*>& states) {
    size_t depth = renderer.getDepth();

    if(depth > renderer.getMaxDepth()) {
        return states[0];
    }
    else
        return Base_op(renderer, states);
}

int main(int argc, char* argv[]) {
    //Parameters
    size_t function_num = 10;
    size_t function_num_stddev = 2;
    size_t function_name_len = 12;
    size_t function_name_stddev = 3;
    size_t statement_list_len = 20;
    size_t statement_list_stddev = 5;
    size_t id_name_len = 10;
    size_t id_name_stddev = 2;
    size_t tree_max_depth = 20;
    size_t int_min_value = 0;
    size_t int_max_value = 10000;

    //Utility nodes
    state_ptr indent(new IndentRenderState());
    state_ptr commit(new CommitRenderState());

    //General strings
    state_ptr newline(new TextRenderState("\n"));
    state_ptr open_cb(new TextRenderState("{\n"));
    state_ptr close_cb(new TextRenderState("}\n"));
    state_ptr semicolon(new TextRenderState(";\n"));
    state_ptr space(new TextRenderState(" "));
    state_ptr assignment(new TextRenderState(" = "));
    state_ptr declare(new TextRenderState(" <- "));
    state_ptr mul(new TextRenderState(" * "));
    state_ptr div(new TextRenderState(" / "));
    state_ptr mod(new TextRenderState(" % "));
    state_ptr open_par(new TextRenderState("("));
    state_ptr close_par(new TextRenderState(")"));

    //Operator choices
    state_ptr mul_ops(new OptionRenderState({mul.get(), div.get(), mod.get()}, uniform_option_picker));

    //Types
    state_ptr keyword_integer(new TextRenderState("int"));
    state_ptr keyword_float(new TextRenderState("float"));
    state_ptr random_datatype(new OptionRenderState({keyword_integer.get(), keyword_float.get()}, uniform_option_picker));

    //Expressions
    //Int expressions
    state_ptr random_int(new RandomIntRenderState(int_min_value, int_max_value));
    state_ptr random_int_id(new IDRenderState(random_int.get(), CATEGORY_INT));
    state_ptr int_parens(new NodeRenderState({open_par.get(), nullptr, close_par.get()}));
    state_ptr int_atom(new OptionRenderState({random_int.get(), random_int_id.get(), int_parens.get()}, depth_based_option_picker<uniform_option_picker>));
    state_ptr int_mul_expression_base(new NodeRenderState({int_atom.get(), mul_ops.get(), nullptr}));
    state_ptr int_mul_expression(new OptionRenderState({int_atom.get(), int_mul_expression_base.get()}, depth_based_option_picker<uniform_option_picker>));
    int_mul_expression_base->setChild(2, int_mul_expression.get());
    state_ptr& int_expression = int_mul_expression;
    int_parens->setChild(1, int_expression.get());

    state_ptr int_gen_new_var(new IDCreateRenderState(id_name_len, id_name_stddev, CATEGORY_INT));
    state_ptr int_makevar_expression(make_noderenderstate(int_gen_new_var, declare, keyword_integer));
    state_ptr opt_int_declare_expression(new OptionRenderState({int_expression.get(), nullptr}, bias_first_option_picker<3>));
    state_ptr int_declare_expression(make_noderenderstate(int_makevar_expression, assignment, opt_int_declare_expression));
    opt_int_declare_expression->setChild(1, int_declare_expression.get());

    //Float expressions
    state_ptr float_declare_expression(new TextRenderState("=f"));
    state_ptr declare_expression(new OptionRenderState({int_declare_expression.get(), float_declare_expression.get()}, uniform_option_picker));

    //Statements
    state_ptr expression_statement_roots(new OptionRenderState({declare_expression.get()}, bias_first_option_picker<9>));
    state_ptr expression_statement(make_noderenderstate(expression_statement_roots, semicolon, commit));
    state_ptr random_statement(new OptionRenderState({expression_statement.get()}, uniform_option_picker));
    state_ptr random_statement_indented(make_noderenderstate(indent, random_statement));

    //Statement lists
    state_ptr statement_list(new RepeatRenderState(random_statement_indented.get(), statement_list_len, statement_list_stddev));

    //Compound statement
    state_ptr compound_statement_scope(new ScopeRenderState(statement_list.get()));
    state_ptr compound_statement(make_noderenderstate(open_cb, compound_statement_scope, close_cb));

    //Function declarations
    state_ptr function_keyword(new TextRenderState("function "));
    state_ptr function_id(new IDCreateRenderState(function_name_len, function_name_stddev));
    state_ptr function_parens(new TextRenderState("() <- "));
    state_ptr function_node(make_noderenderstate(function_keyword, function_id, function_parens, random_datatype, space, compound_statement, newline));

    //Global scope 
    state_ptr root_node(new RepeatRenderState(function_node.get(), function_num, function_num_stddev));

    std::random_device seed_device;
    CodeRenderer renderer(root_node.get(), seed_device());
    renderer.setMaxDepth(tree_max_depth);
    renderer.enterScope();
    renderer.render(std::cout);
    renderer.exitScope();
    return 0;
}