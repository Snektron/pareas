#ifndef _PAREAS_LPG_TOKEN_MAPPING_HPP
#define _PAREAS_LPG_TOKEN_MAPPING_HPP

#include <string_view>
#include <unordered_map>
#include <cstddef>

namespace pareas {
    using TokenIdMap = std::unordered_map<std::string_view, size_t>;

    class TokenMapping {
        std::unordered_map<std::string_view, size_t> token_ids;

    public:
        explicit TokenMapping(TokenIdMap&& token_ids);
        bool contains(std::string_view token_name) const;
    };
}

#endif
