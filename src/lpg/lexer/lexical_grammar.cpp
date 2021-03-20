#include "pareas/lpg/lexer/lexical_grammar.hpp"

#include <cassert>

namespace pareas::lexer {
    Token Lexeme::as_token() const {
        return {Token::Type::USER_DEFINED, this->name};
    }

    size_t LexicalGrammar::lexeme_id(const Lexeme* lexeme) const {
        assert(lexeme >= this->lexemes.data() && lexeme < &this->lexemes.data()[this->lexemes.size()]);
        return lexeme - this->lexemes.data();
    }

    void LexicalGrammar::add_tokens(TokenMapping& tm) const {
        tm.insert(Token::INVALID);
        for (const auto& lexeme : this->lexemes) {
            tm.insert(lexeme.as_token());
        }
    }
}
