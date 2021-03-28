#include "pareas/lpg/lexer/render.hpp"
#include "pareas/lpg/futhark_dataset.hpp"

#include <fmt/ostream.h>

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
        this->r->align_dat(sizeof(EncodedTransition));
        auto offset = this->r->dat_offset();

        uint64_t dim = this->lexer->initial_states.size();
        for (uint64_t i = 0; i < dim; ++i) {
            const auto& transition = this->lexer->initial_states[i];
            auto encoded = this->encode(transition);
            this->r->dat.write(reinterpret_cast<const char*>(&encoded), sizeof encoded);
        }

        return offset;
    }

    size_t LexerRenderer::render_merge_table_data() const {
        this->r->align_dat(sizeof(EncodedTransition));
        auto offset = this->r->dat_offset();

        const auto& merge_table = this->lexer->merge_table;

        uint64_t dim = merge_table.states();
        // Make sure to iterate in right order
        for (uint64_t x = 0; x < dim; ++x) {
            for (uint64_t y = 0; y < dim; ++y) {
                auto encoded = this->encode(merge_table(x, y));

                this->r->dat.write(reinterpret_cast<const char*>(&encoded), sizeof encoded);
            }
        }

        return offset;
    }

    size_t LexerRenderer::render_final_state_data() const {
        switch (this->tm->backing_type_bits()) {
            case 8:
                return this->render_final_state_data_with_type<uint8_t>();
            case 16:
                return this->render_final_state_data_with_type<uint16_t>();
            case 32:
                return this->render_final_state_data_with_type<uint32_t>();
            case 64:
                return this->render_final_state_data_with_type<uint64_t>();
            default:
                assert(false);
        }
    }

    template <typename T>
    size_t LexerRenderer::render_final_state_data_with_type() const {
        this->r->align_dat(sizeof(T));
        auto offset = this->r->dat_offset();

        uint64_t dim = this->lexer->final_states.size();
        auto data = futhark::Array<T>({dim});

        for (uint64_t i = 0; i < dim; ++i) {
            const auto* lexeme = this->lexer->final_states[i];
            if (!lexeme) {
                data.at(futhark::Index(&i, 1)) = this->tm->token_id(Token::INVALID);
            } else {
                data.at(futhark::Index(&i, 1)) = this->tm->token_id(lexeme->as_token());
            }
        }

        data.write(this->r->dat);
        return offset;
    }

    auto LexerRenderer::encode(const ParallelLexer::Transition& t) const -> EncodedTransition {
        assert(t.result_state < PRODUCES_TOKEN_MASK);
        return t.result_state | (t.produces_lexeme ? PRODUCES_TOKEN_MASK : 0);
    }
}
