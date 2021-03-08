#include "pareas/lpg/lexer/lexical_grammar.hpp"

#include <cassert>

namespace pareas::lexer {
    size_t LexicalGrammar::token_id(const Token* token) const {
        assert(token >= this->tokens.data() && token < &this->tokens.data()[this->tokens.size()]);
        return token - this->tokens.data();
    }
}
