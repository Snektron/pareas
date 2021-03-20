#include "pareas/lpg/lexer/lexical_grammar.hpp"

#include <cassert>

namespace pareas::lexer {
    size_t LexicalGrammar::lexeme_id(const Lexeme* lexeme) const {
        assert(lexeme >= this->lexemes.data() && lexeme < &this->lexemes.data()[this->lexemes.size()]);
        return lexeme - this->lexemes.data();
    }

    TokenMapping LexicalGrammar::build_token_mapping() const {
        auto token_ids = TokenIdMap();
        for (const auto& lexeme : this->lexemes) {
            token_ids.insert({std::string(lexeme.name), this->lexeme_id(&lexeme)});
        }
        return TokenMapping(std::move(token_ids));
    }
}
