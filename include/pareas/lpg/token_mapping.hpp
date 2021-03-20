#ifndef _PAREAS_LPG_TOKEN_MAPPING_HPP
#define _PAREAS_LPG_TOKEN_MAPPING_HPP

#include <string>
#include <unordered_map>
#include <iosfwd>
#include <cstddef>

namespace pareas {
    // struct Token {
    //     enum class Type {
    //         USER_DEFINED,
    //         INVALID,
    //         START_OF_INPUT,
    //         END_OF_INPUT
    //     };

    //     static const Token INVALID;
    //     static const Token START_OF_INPUT;
    //     static const Token END_OF_INPUT;

    //     Type type;
    //     std::string name;

    //     struct Hash {
    //         size_t operator()(const Token& token) const;
    //     };
    // };

    // bool operator==(const Token& lhs, const Token& rhs);

    using TokenIdMap = std::unordered_map<std::string, size_t>;

    class TokenMapping {
        std::unordered_map<std::string, size_t> token_ids;

    public:
        explicit TokenMapping(TokenIdMap&& token_ids);
        bool contains(const std::string& token_name) const;
        size_t backing_type_bits() const;
        void render_futhark(std::ostream& out) const;
        size_t token_id(const std::string& token_name) const;
        size_t num_tokens() const;
    };
}

#endif
