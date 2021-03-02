#include "pareas/lexgen/render.hpp"
#include "pareas/common/futhark_dataset.hpp"
#include "pareas/common/render_util.hpp"

#include <fmt/ostream.h>

#include <iterator>

namespace pareas {
    LexerRenderer::LexerRenderer(std::span<const Token> tokens, const ParallelLexer* lexer):
        tokens(tokens), lexer(lexer), token_bits(int_bit_width(tokens.size())) {
    }

    void LexerRenderer::render_code(std::ostream& out) const {
        fmt::print("module token = u{}\n", this->token_bits);
        for (size_t i = 0; i < tokens.size(); ++i) {
            fmt::print("let token_{}: token.t = {}\n", tokens[i].name, i);
        }
        fmt::print("let num_tokens: i64 = {}\n", tokens.size());
        fmt::print("let identity_index: i64 = {}\n", this->lexer->identity_state_index);
    }

    void LexerRenderer::render_initial_state_dataset(std::ostream& out) const {
        auto data = std::vector<EncodedTransition>();
        for (const ParallelLexer::Transition t : this->lexer->initial_states) {
            data.push_back(this->encode(t));
        }

        uint64_t dim[] = {data.size()};
        write_futhark_array(out, dim, std::span<const EncodedTransition>(data.begin(), data.end()));
    }

    void LexerRenderer::render_merge_table_dataset(std::ostream& out) const {
        const auto& merge_table = this->lexer->merge_table;

        auto data = std::vector<EncodedTransition>();
        for (size_t y = 0; y < merge_table.states(); ++y) {
            for (size_t x = 0; x < merge_table.states(); ++x) {
                data.push_back(encode(merge_table(x, y)));
            }
        }

        uint64_t dim[] = {merge_table.states(), merge_table.states()};
        write_futhark_array(out, dim, std::span<const EncodedTransition>(data.begin(), data.end()));
    }

    void LexerRenderer::render_final_state_dataset(std::ostream& out) const {
        assert(this->token_bits == 8); // TODO: Figure out a better way to generate datasets

        auto data = std::vector<uint8_t>();
        for (const auto* token : this->lexer->final_states) {
            if (!token) {
                data.push_back(0);
            } else {
                auto offset = token - &this->tokens.front(); // TODO: Improve this
                data.push_back(static_cast<uint8_t>(offset));
            }
        }

        uint64_t dim[] = {data.size()};
        write_futhark_array(out, dim, std::span<const uint8_t>(data.begin(), data.end()));
    }

    auto LexerRenderer::encode(const ParallelLexer::Transition& t) const -> EncodedTransition {
        assert(t.result_state < PRODUCES_TOKEN_MASK);
        return t.result_state | (t.produces_token ? PRODUCES_TOKEN_MASK : 0);
    }
}
