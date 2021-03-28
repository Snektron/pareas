#ifndef _PAREAS_LPG_LEXER_RENDER_HPP
#define _PAREAS_LPG_LEXER_RENDER_HPP

#include "pareas/lpg/renderer.hpp"
#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/lexer/parallel_lexer.hpp"

#include <limits>
#include <span>
#include <iosfwd>
#include <cstdint>

namespace pareas::lexer {
    class LexerRenderer {
        using EncodedTransition = uint16_t;
        constexpr const static auto ENCODED_TRANSITION_BITS = std::numeric_limits<EncodedTransition>::digits;
        constexpr const static auto PRODUCES_TOKEN_MASK = 1 << (ENCODED_TRANSITION_BITS - 1);

        Renderer* r;
        const TokenMapping* tm;
        const ParallelLexer* lexer;

    public:
        LexerRenderer(Renderer* r, const TokenMapping* tm, const ParallelLexer* lexer);
        void render() const;

    private:
        void render_initial_state_data() const;
        void render_merge_table_data() const;
        void render_final_state_data() const;

        template <typename T>
        void render_final_state_data_with_type() const;

        EncodedTransition encode(const ParallelLexer::Transition& t) const;
    };
}

#endif
