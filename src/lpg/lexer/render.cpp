#include "pareas/lpg/lexer/render.hpp"

#include <fmt/ostream.h>

#include <cassert>

namespace pareas::lexer {
    LexerRenderer::LexerRenderer(Renderer* r, const TokenMapping* tm, const ParallelLexer* lexer):
        r(r), tm(tm), lexer(lexer) {
    }

    void LexerRenderer::render() const {
        assert(this->lexer->merge_table.states() == this->lexer->final_states.size());

        fmt::print(this->r->fut, "let identity_state: u{} = {}\n", ENCODED_TRANSITION_BITS, this->lexer->identity_state_index);

        fmt::print(
            this->r->hpp,
            "struct LexTable {{\n"
            "    using State = uint16_t;\n"
            "    static constexpr const size_t NUM_INITIAL_STATES = 256;\n"
            "    size_t n;\n"
            "    const State* initial_states; // NUM_INITIAL_STATES\n"
            "    const State* merge_table; // n * n\n"
            "    const Token* final_states; // n\n"
            "}};\n"
            "extern const LexTable lex_table;\n"
        );

        auto initial_state_offset = this->render_initial_state_data();
        auto merge_table_offset = this->render_merge_table_data();
        auto final_state_offset = this->render_final_state_data();

        fmt::print(
            this->r->cpp,
            "const LexTable lex_table = {{\n"
            "    .n = {},\n"
            "    .initial_states = {},\n"
            "    .merge_table = {},\n"
            "    .final_states = {}\n"
            "}};\n",
            this->lexer->merge_table.states(),
            this->r->render_offset_cast(initial_state_offset, "LexTable::State"),
            this->r->render_offset_cast(merge_table_offset, "LexTable::State"),
            this->r->render_offset_cast(final_state_offset, "Token")
        );
    }

    size_t LexerRenderer::render_initial_state_data() const {
        this->r->align_data(sizeof(EncodedTransition));
        auto offset = this->r->data_offset();

        uint64_t dim = this->lexer->initial_states.size();
        for (uint64_t i = 0; i < dim; ++i) {
            const auto& transition = this->lexer->initial_states[i];
            auto encoded = this->encode(transition);
            this->r->write_data_int(encoded, sizeof(EncodedTransition));
        }

        return offset;
    }

    size_t LexerRenderer::render_merge_table_data() const {
        this->r->align_data(sizeof(EncodedTransition));
        auto offset = this->r->data_offset();

        const auto& merge_table = this->lexer->merge_table;

        uint64_t dim = merge_table.states();
        // Make sure to iterate in right order
        for (uint64_t x = 0; x < dim; ++x) {
            for (uint64_t y = 0; y < dim; ++y) {
                auto encoded = this->encode(merge_table(x, y));
                this->r->write_data_int(encoded, sizeof(EncodedTransition));
            }
        }

        return offset;
    }

    size_t LexerRenderer::render_final_state_data() const {
        this->r->align_data(this->tm->backing_type_bits() / 8);
        auto offset = this->r->data_offset();

        uint64_t dim = this->lexer->final_states.size();
        for (uint64_t i = 0; i < dim; ++i) {
            const auto* lexeme = this->lexer->final_states[i];

            uint64_t token_definition = lexeme ? this->tm->token_id(lexeme->as_token()) : this->tm->token_id(Token::INVALID);
            this->r->write_data_int(token_definition, this->tm->backing_type_bits() / 8);
        }

        return offset;
    }

    auto LexerRenderer::encode(const ParallelLexer::Transition& t) const -> EncodedTransition {
        assert(t.result_state < PRODUCES_TOKEN_MASK);
        return t.result_state | (t.produces_lexeme ? PRODUCES_TOKEN_MASK : 0);
    }
}
