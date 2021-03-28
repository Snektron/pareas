#include "pareas/lpg/lexer/render.hpp"
#include "pareas/lpg/futhark_dataset.hpp"

#include <fmt/ostream.h>

namespace pareas::lexer {
    LexerRenderer::LexerRenderer(Renderer* r, const TokenMapping* tm, const ParallelLexer* lexer):
        r(r), tm(tm), lexer(lexer) {
    }

    void LexerRenderer::render() const {
        fmt::print(this->r->fut, "let identity_state: u{} = {}\n", ENCODED_TRANSITION_BITS, this->lexer->identity_state_index);

        this->render_initial_state_data();
        this->render_merge_table_data();
        this->render_final_state_data();
    }

    void LexerRenderer::render_initial_state_data() const {
        uint64_t dim = this->lexer->initial_states.size();
        auto data = futhark::Array<EncodedTransition>({dim});

        for (uint64_t i = 0; i < dim; ++i) {
            const auto& transition = this->lexer->initial_states[i];
            data.at(futhark::Index(&i, 1)) = this->encode(transition);
        }

        data.write(this->r->dat);
    }

    void LexerRenderer::render_merge_table_data() const {
        const auto& merge_table = this->lexer->merge_table;

        uint64_t dim = merge_table.states();
        auto data = futhark::Array<EncodedTransition>({dim, dim});

        for (uint64_t y = 0; y < merge_table.states(); ++y) {
            for (uint64_t x = 0; x < merge_table.states(); ++x) {
                uint64_t index[] = {x, y};
                data.at(futhark::Index(index)) = this->encode(merge_table(x, y));
            }
        }

        data.write(this->r->dat);
    }

    void LexerRenderer::render_final_state_data() const {
        switch (this->tm->backing_type_bits()) {
            case 8:
                this->render_final_state_data_with_type<uint8_t>();
                return;
            case 16:
                this->render_final_state_data_with_type<uint16_t>();
                return;
            case 32:
                this->render_final_state_data_with_type<uint32_t>();
                return;
            case 64:
                this->render_final_state_data_with_type<uint64_t>();
                return;
            default:
                assert(false);
        }
    }

    template <typename T>
    void LexerRenderer::render_final_state_data_with_type() const {
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
    }

    auto LexerRenderer::encode(const ParallelLexer::Transition& t) const -> EncodedTransition {
        assert(t.result_state < PRODUCES_TOKEN_MASK);
        return t.result_state | (t.produces_lexeme ? PRODUCES_TOKEN_MASK : 0);
    }
}
