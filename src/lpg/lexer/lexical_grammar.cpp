#include "pareas/lpg/lexer/lexical_grammar.hpp"

#include <cassert>

namespace pareas::lexer {
    size_t LexicalGrammar::token_id(const Token* token) const {
        assert(token >= this->tokens.data() && token < &this->tokens.data()[this->tokens.size()]);
        return token - this->tokens.data();
    }

    TokenMapping LexicalGrammar::build_token_mapping() const {
        auto token_ids = TokenIdMap();
        for (const auto& token : this->tokens) {
            token_ids.insert({std::string(token.name), this->token_id(&token)});
        }
        return TokenMapping(std::move(token_ids));
    }
}
