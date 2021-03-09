#include "pareas/lpg/token_mapping.hpp"

#include <utility>

namespace pareas {
    TokenMapping::TokenMapping(TokenIdMap&& token_ids):
        token_ids(std::move(token_ids)) {}

    bool TokenMapping::contains(std::string_view token_name) const {
        return this->token_ids.contains(token_name);
    }
}