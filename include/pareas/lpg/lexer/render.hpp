#ifndef _PAREAS_LPG_LEXER_RENDER_HPP
#define _PAREAS_LPG_LEXER_RENDER_HPP

#include "pareas/lpg/lexer/parallel_lexer.hpp"
#include "pareas/lpg/lexer/lexical_grammar.hpp"

#include <limits>
#include <span>
#include <iosfwd>
#include <cstdint>

namespace pareas::lexer {
    class LexerRenderer {
        using EncodedTransition = uint16_t;
        constexpr const static auto ENCODED_TRANSITION_BITS = std::numeric_limits<EncodedTransition>::digits;
        constexpr const static auto PRODUCES_TOKEN_MASK = 1 << (ENCODED_TRANSITION_BITS - 1);

        const LexicalGrammar* g;
        const ParallelLexer* lexer;
        size_t token_bits;

    public:
        LexerRenderer(const LexicalGrammar* g, const ParallelLexer* lexer);

        void render_code(std::ostream& out) const;
        void render_initial_state_dataset(std::ostream& out) const;
        void render_merge_table_dataset(std::ostream& out) const;
        void render_final_state_dataset(std::ostream& out) const;

    private:
        EncodedTransition encode(const ParallelLexer::Transition& t) const;
    };
}

#endif
