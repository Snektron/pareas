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

    void TokenMapping::render(Renderer& r) const {
        fmt::print(r.fut, "module token = u{}\n", this->backing_type_bits());

        fmt::print(r.hpp, "enum class Token : uint{}_t {{\n", this->backing_type_bits());

        fmt::print(r.cpp, "const char* token_name(Token t) {{\n");
        fmt::print(r.cpp, "    switch (t) {{\n");

        // Render the tokens nice and ordered.
        auto tokens_ordered = std::vector<const Token*>(this->num_tokens());
        for (const auto& [token, id] : this->tokens)
            tokens_ordered[id] = &token;

        for (size_t id = 0; id < tokens_ordered.size(); ++id) {
            const auto& name = tokens_ordered[id]->name;

            auto name_upper = tokens_ordered[id]->name;
            std::transform(name_upper.begin(), name_upper.end(), name_upper.begin(), ::toupper);

            bool special = tokens_ordered[id]->type == Token::Type::USER_DEFINED;

            fmt::print(
                r.fut,
                "let {}token_{}: token.t = {}\n",
                special ? "" : "special_",
                name,
                id
            );

            fmt::print(
                r.hpp,
                "    {}{} = {},\n",
                special ? "" : "SPECIAL_",
                name_upper,
                id
            );

            fmt::print(
                r.cpp,
                "        case Token::{}{}: return \"{}\";\n",
                special ? "" : "SPECIAL_",
                name_upper,
                name,
                id
            );
        }

        fmt::print(r.fut, "let num_tokens: i64 = {}\n", this->num_tokens());

        fmt::print(r.hpp, "}};\n");
        fmt::print(r.hpp, "constexpr const size_t NUM_TOKENS = {};\n", this->num_tokens());
        fmt::print(r.hpp, "const char* token_name(Token t);\n");

        fmt::print(r.cpp, "    }}\n}}\n");
    }
}
