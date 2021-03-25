#include "test/coderenderer.hpp"
#include "test/renderstate.hpp"
#include "test/noderenderstate.hpp"
#include "test/textrenderstate.hpp"
#include "test/repeatrenderstate.hpp"
#include "test/optionrenderstate.hpp"
#include "test/idcreaterenderstate.hpp"
#include "test/indentrenderstate.hpp"
#include "test/scoperenderstate.hpp"

#include <memory>
#include <iostream>

using state_ptr = std::unique_ptr<RenderState>;

template <typename... T>
NodeRenderState* make_noderenderstate(const T&... args) {
    return new NodeRenderState({(args.get())...});
}

const RenderState* uniform_option_picker(CodeRenderer& renderer, const std::vector<const RenderState*> states) {
    std::uniform_int_distribution<size_t> distr(0, states.size() - 1);

    return states[distr(renderer.get_rng())];
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

    //Utility nodes
    state_ptr indent(new IndentRenderState());

    //General strings
    state_ptr newline(new TextRenderState("\n"));
    state_ptr open_par(new TextRenderState("{\n"));
    state_ptr close_par(new TextRenderState("}\n"));
    state_ptr semicolon(new TextRenderState(";\n"));
    state_ptr space(new TextRenderState(" "));

    //Types
    state_ptr keyword_integer(new TextRenderState("int"));
    state_ptr keyword_float(new TextRenderState("float"));
    state_ptr random_datatype(new OptionRenderState({keyword_integer.get(), keyword_float.get()}, uniform_option_picker));

    //Statements
    state_ptr random_id(new IDCreateRenderState(id_name_len, id_name_stddev));
    state_ptr expression_statement(make_noderenderstate(random_id, semicolon));
    state_ptr random_statement(new OptionRenderState({expression_statement.get()}, uniform_option_picker));
    state_ptr random_statement_indented(make_noderenderstate(indent, random_statement));

    //Statement lists
    state_ptr statement_list(new RepeatRenderState(random_statement_indented.get(), statement_list_len, statement_list_stddev));

    //Compound statement
    state_ptr compound_statement_scope(new ScopeRenderState(statement_list.get()));
    state_ptr compound_statement(make_noderenderstate(open_par, compound_statement_scope, close_par));

    //Function declarations
    state_ptr function_keyword(new TextRenderState("function "));
    state_ptr function_id(new IDCreateRenderState(function_name_len, function_name_stddev));
    state_ptr function_parens(new TextRenderState("() <- "));
    state_ptr function_node(make_noderenderstate(function_keyword, function_id, function_parens, random_datatype, space, compound_statement, newline));

    //Global scope 
    state_ptr root_node(new RepeatRenderState(function_node.get(), function_num, function_num_stddev));

    std::random_device seed_device;
    CodeRenderer renderer(root_node.get(), seed_device());
    renderer.enterScope();
    renderer.render(std::cout);
    renderer.exitScope();
    return 0;
}