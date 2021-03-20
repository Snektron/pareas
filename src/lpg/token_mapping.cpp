#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/render_util.hpp"
#include "pareas/lpg/hash_util.hpp"

#include <fmt/ostream.h>

#include <string_view>
#include <vector>
#include <utility>

namespace pareas {
    // const Token Token::INVALID = {Type::INVALID, "invalid"};
    // const Token Token::START_OF_INPUT = {Type::START_OF_INPUT, "soi"};
    // const Token Token::END_OF_INPUT = {Type::END_OF_INPUT, "eoi"};

    // size_t Token::Hash::operator()(const Token& token) const {
    //     return hash_combine(std::hash<Type>{}(token.type), std::hash<std::string>{}(token.name));
    // };

    // bool operator==(const Token& rhs, const Token& lhs) const {
    //     return rhs.token == lhs.token && rhs.name == rhs.token;
    // }

    TokenMapping::TokenMapping(TokenIdMap&& token_ids):
        token_ids(std::move(token_ids)) {}

    bool TokenMapping::contains(const std::string& token_name) const {
        return this->token_ids.contains(token_name);
    }

    size_t TokenMapping::backing_type_bits() const {
        return int_bit_width(this->token_ids.size() - 1);
    }

    void TokenMapping::render_futhark(std::ostream& out) const {
        fmt::print(out, "module token = u{}\n", this->backing_type_bits());

        // Render the tokens nice and ordered.
        auto tokens_ordered = std::vector<std::string_view>(this->token_ids.size());
        for (const auto& [name, id] : this->token_ids) {
            tokens_ordered[id] = name;
        }

        for (size_t id = 0; id < tokens_ordered.size(); ++id) {
            fmt::print(out, "let token_{}: token.t = {}\n", tokens_ordered[id], id);
        }

        fmt::print(out, "let num_tokens: i64 = {}\n", tokens_ordered.size());
    }

    size_t TokenMapping::token_id(const std::string& token_name) const {
        return this->token_ids.at(token_name);
    }

    size_t TokenMapping::num_tokens() const {
        return this->token_ids.size();
    }
}
