#include "pareas/lpg/lexer/render.hpp"
#include "pareas/lpg/futhark_dataset.hpp"

#include <fmt/ostream.h>

namespace pareas::lexer {
    Renderer::Renderer(const TokenMapping* tm, const ParallelLexer* lexer):
        tm(tm), lexer(lexer) {
    }

    void Renderer::render_futhark(std::ostream& out) const {
        fmt::print(out, "let identity_state: u{} = {}\n", ENCODED_TRANSITION_BITS, this->lexer->identity_state_index);
    }

    void Renderer::render_initial_state_data(std::ostream& out) const {
        uint64_t dim = this->lexer->initial_states.size();
        auto data = futhark::Array<EncodedTransition>({dim});

        for (uint64_t i = 0; i < dim; ++i) {
            const auto& transition = this->lexer->initial_states[i];
            data.at(futhark::Index(&i, 1)) = this->encode(transition);
        }

        data.write(out);
    }

    void Renderer::render_merge_table_data(std::ostream& out) const {
        const auto& merge_table = this->lexer->merge_table;

        uint64_t dim = merge_table.states();
        auto data = futhark::Array<EncodedTransition>({dim, dim});

        for (uint64_t y = 0; y < merge_table.states(); ++y) {
            for (uint64_t x = 0; x < merge_table.states(); ++x) {
                uint64_t index[] = {x, y};
                data.at(futhark::Index(index)) = this->encode(merge_table(x, y));
            }
        }

        data.write(out);
    }

    void Renderer::render_final_state_data(std::ostream& out) const {
        switch (this->tm->backing_type_bits()) {
            case 8:
                this->render_final_state_data_with_type<uint8_t>(out);
                return;
            case 16:
                this->render_final_state_data_with_type<uint16_t>(out);
                return;
            case 32:
                this->render_final_state_data_with_type<uint32_t>(out);
                return;
            case 64:
                this->render_final_state_data_with_type<uint64_t>(out);
                return;
            default:
                assert(false);
        }
    }

    template <typename T>
    void Renderer::render_final_state_data_with_type(std::ostream& out) const {
        uint64_t dim = this->lexer->final_states.size();
        auto data = futhark::Array<T>({dim});

        for (uint64_t i = 0; i < dim; ++i) {
            const auto* token = this->lexer->final_states[i];
            if (!token) {
                data.at(futhark::Index(&i, 1)) = 0;
            } else {
                data.at(futhark::Index(&i, 1)) = this->tm->token_id(std::string(token->name));
            }
        }

        data.write(out);
    }

    auto Renderer::encode(const ParallelLexer::Transition& t) const -> EncodedTransition {
        assert(t.result_state < PRODUCES_TOKEN_MASK);
        return t.result_state | (t.produces_token ? PRODUCES_TOKEN_MASK : 0);
    }
}
