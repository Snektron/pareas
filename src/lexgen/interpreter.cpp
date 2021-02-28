#include "pareas/lexgen/interpreter.hpp"

#include <fmt/format.h>

#include <vector>

namespace pareas {
    LexerInterpreter::LexerInterpreter(const ParallelLexer* lexer):
        lexer(lexer) {}

    void LexerInterpreter::lex_linear(std::string_view input) const {
        auto states = std::vector<ParallelLexer::StateIndex>();

        fmt::print("-- init --\n");

        for (auto c : input) {
            auto state = this->lexer->initial_states[c];
            states.push_back(state);
            fmt::print("{} -> {}\n", c, state);
        }

        fmt::print("-- lex --\n");
        for (size_t i = 1; i < input.size(); ++i) {
            auto prev = states[i - 1];
            auto state = this->lexer->merge_table(states[i - 1], states[i]);
            states[i] = state.result_state;
            if (state.produces_token) {
                auto t = this->lexer->final_states[prev];
                fmt::print("{}\n", t ? t->name : "(internal error)");
            }
        }

        auto t = this->lexer->final_states[states.back()];
        fmt::print("{}\n", t ? t->name : "(input error)");
    }
}
