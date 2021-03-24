#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/render_util.hpp"
#include "pareas/lpg/hash_util.hpp"

#include <fmt/ostream.h>

#include <string_view>
#include <vector>
#include <utility>
#include <ostream>

namespace pareas {
    const Token Token::INVALID = {Type::INVALID, "invalid"};
    const Token Token::START_OF_INPUT = {Type::START_OF_INPUT, "soi"};
    const Token Token::END_OF_INPUT = {Type::END_OF_INPUT, "eoi"};

    size_t Token::Hash::operator()(const Token& token) const {
        return hash_combine(std::hash<Type>{}(token.type), std::hash<std::string>{}(token.name));
    };

    bool operator==(const Token& rhs, const Token& lhs) {
        return rhs.type == lhs.type && rhs.name == rhs.name;
    }

    std::ostream& operator<<(std::ostream& os, const Token& token) {
        switch (token.type) {
            case Token::Type::USER_DEFINED:
                break;
            case Token::Type::INVALID:
            case Token::Type::START_OF_INPUT:
            case Token::Type::END_OF_INPUT:
                os << "special_";
                break;
        }

        return os << "token_" << token.name;
    }

    void TokenMapping::insert(const Token& token) {
        this->tokens.insert({token, this->num_tokens()});
    }

    bool TokenMapping::contains(const Token& token) const {
        return this->tokens.contains(token);
    }

    size_t TokenMapping::backing_type_bits() const {
        return int_bit_width(this->tokens.size() - 1);
    }

    void TokenMapping::render_futhark(std::ostream& out) const {
        fmt::print(out, "module token = u{}\n", this->backing_type_bits());

        // Render the tokens nice and ordered.
        auto tokens_ordered = std::vector<const Token*>(this->num_tokens());
        for (const auto& [token, id] : this->tokens) {
            tokens_ordered[id] = &token;
        }

        for (size_t id = 0; id < tokens_ordered.size(); ++id) {
            fmt::print(out, "let {}: token.t = {}\n", *tokens_ordered[id], id);
        }

        fmt::print(out, "let num_tokens: i64 = {}\n", this->num_tokens());
    }

    size_t TokenMapping::token_id(const Token& token) const {
        return this->tokens.at(token);
    }

    size_t TokenMapping::num_tokens() const {
        return this->tokens.size();
    }
}
