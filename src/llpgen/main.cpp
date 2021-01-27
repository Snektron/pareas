#include "llpgen/grammar.hpp"
#include "llpgen/generator.hpp"

#include <iostream>

using literals::operator ""_t;
using literals::operator ""_nt;

int main() {
    auto test_grammar = Grammar("S"_nt, "⊢"_t, "⊣"_t);
    test_grammar.add_rule({"S"_nt, {"⊢"_t, "E"_nt, "⊣"_t}});
    test_grammar.add_rule({"E"_nt, {"T"_nt, "E'"_nt}});
    test_grammar.add_rule({"E'"_nt, {"+"_t, "T"_nt, "E'"_nt}});
    test_grammar.add_rule({"E'"_nt, {}});
    test_grammar.add_rule({"T"_nt, {"a"_t}});
    test_grammar.add_rule({"T"_nt, {"["_t, "E"_nt, "]"_t}});

    // auto test_grammar = Grammar("S"_nt, "⊢"_t, "⊣"_t);
    // test_grammar.add_rule({"S"_nt, {"⊢"_t, "E"_nt, "⊣"_t}});
    // test_grammar.add_rule({"E"_nt, {"T"_nt, "E'"_nt}});
    // test_grammar.add_rule({"E'"_nt, {"+"_t, "T"_nt, "E'"_nt}});
    // test_grammar.add_rule({"E'"_nt, {}});
    // test_grammar.add_rule({"T"_nt, {"F"_nt, "T'"_nt}});
    // test_grammar.add_rule({"T'"_nt, {"*"_t, "F"_nt, "T'"_nt}});
    // test_grammar.add_rule({"T'"_nt, {}});
    // test_grammar.add_rule({"F"_nt, {"id"_t}});
    // test_grammar.add_rule({"F"_nt, {"("_t, "E"_nt, ")"_t}});

    auto gen = LLPGenerator(&test_grammar);
    auto psls = gen.build_psls_table();
    // psls.dump_csv(std::cout);
}
