#ifndef _PAREAS_LPG_LEXER_RENDER_HPP
#define _PAREAS_LPG_LEXER_RENDER_HPP

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

        const TokenMapping* tm;
        const ParallelLexer* lexer;

    public:
        LexerRenderer(const TokenMapping* tm, const ParallelLexer* lexer);

        void render_code(std::ostream& out) const;
        void render_initial_state_dataset(std::ostream& out) const;
        void render_merge_table_dataset(std::ostream& out) const;
        void render_final_state_dataset(std::ostream& out) const;

        template <typename T>
        void render_final_state_dataset_with_type(std::ostream& out) const;

    private:
        EncodedTransition encode(const ParallelLexer::Transition& t) const;
    };
}

#endif
