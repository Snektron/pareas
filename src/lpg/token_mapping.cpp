#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/render_util.hpp"
#include "pareas/lpg/hash_util.hpp"

#include <fmt/ostream.h>

#include <algorithm>
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

    void TokenMapping::insert(const Token& token) {
        this->tokens.insert({token, this->num_tokens()});
    }

    bool TokenMapping::contains(const Token& token) const {
        return this->tokens.contains(token);
    }

    size_t TokenMapping::backing_type_bits() const {
        return int_bit_width(this->tokens.size() - 1);
    }

    size_t TokenMapping::token_id(const Token& token) const {
        return this->tokens.at(token);
    }

    size_t TokenMapping::num_tokens() const {
        return this->tokens.size();
    }

    void TokenMapping::render_futhark(std::ostream& out) const {
        fmt::print(out, "module token = u{}\n", this->backing_type_bits());

        // Render the tokens nice and ordered.
        auto tokens_ordered = std::vector<const Token*>(this->num_tokens());
        for (const auto& [token, id] : this->tokens)
            tokens_ordered[id] = &token;

        for (size_t id = 0; id < tokens_ordered.size(); ++id) {
            fmt::print(
                out,
                tokens_ordered[id]->type == Token::Type::USER_DEFINED ? "" : "special_",
                "let {}token_{}: token.t = {}\n",
                tokens_ordered[id]->name,
                id
            );
        }

        fmt::print(out, "let num_tokens: i64 = {}\n", this->num_tokens());
    }

    void TokenMapping::render_cpp_header(std::ostream& out) const {
        fmt::print(out, "    enum class Token : uint{}_t {{\n", this->backing_type_bits());

        // Render the tokens nice and ordered.
        auto tokens_ordered = std::vector<const Token*>(this->num_tokens());
        for (const auto& [token, id] : this->tokens)
            tokens_ordered[id] = &token;

        for (size_t id = 0; id < tokens_ordered.size(); ++id) {
            auto name = tokens_ordered[id]->name;
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);

            fmt::print(
                out,
                "        {}{} = {},\n",
                tokens_ordered[id]->type == Token::Type::USER_DEFINED ? "" : "SPECIAL_",
                name,
                id
            );
        }

        fmt::print(out, "    }};\n");
        fmt::print(out, "    const char* token_name(Token t);\n");
    }

    void TokenMapping::render_cpp_source(std::ostream& out) const {
        fmt::print(
            out,
            "const char* token_name(Token t) {{\n"
            "    switch (t) {{\n"
        );

        // Render the tokens nice and ordered.
        auto tokens_ordered = std::vector<const Token*>(this->num_tokens());
        for (const auto& [token, id] : this->tokens)
            tokens_ordered[id] = &token;

        for (size_t id = 0; id < tokens_ordered.size(); ++id) {
            auto name = tokens_ordered[id]->name;
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);

            fmt::print(
                out,
                "        case Token::{}{}: return \"{}\";\n",
                tokens_ordered[id]->type == Token::Type::USER_DEFINED ? "" : "SPECIAL_",
                name,
                tokens_ordered[id]->name,
                id
            );
        }

        fmt::print(out, "    }}\n}}\n");
    }
}
