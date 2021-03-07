#include "pareas/lpg/parser/llp/render.hpp"
#include "pareas/lpg/parser/llp/admissible_pair.hpp"
#include "pareas/lpg/render_util.hpp"

#include <fmt/ostream.h>

#include <bit>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <cstdint>
#include <cassert>

namespace {
    using namespace pareas::parser;
    using namespace pareas::parser::llp;

    struct String {
        int32_t offset;
        int32_t size;
    };

    template <typename T>
    struct StringTable {
        std::vector<T> superstring;
        std::unordered_map<AdmissiblePair, String, AdmissiblePair::Hash> strings;

        template <typename F>
        StringTable(const ParsingTable& pt, F get_string);

        void render(
            std::ostream& out,
            const std::string& base_name,
            const std::string& table_type,
            const std::unordered_map<Terminal, size_t, Terminal::Hash>& token_mapping
        );
    };

    template <typename T>
    template <typename F>
    StringTable<T>::StringTable(const ParsingTable& pt, F get_string) {
        // Simple implementation for now
        int32_t offset = 0;
        for (const auto& [ap, entry] : pt.table) {
            auto string = get_string(entry);
            this->superstring.insert(superstring.end(), string.begin(), string.end());
            this->strings[ap] = {offset, static_cast<int32_t>(string.size())};
            offset += string.size();
        }
    }

    template <typename T>
    void StringTable<T>::render(
        std::ostream& out,
        const std::string& base_name,
        const std::string& table_type,
        const std::unordered_map<Terminal, size_t, Terminal::Hash>& token_mapping
    ) {
        // Multiply by 2 to account for the sign bit
        size_t offset_bits = pareas::int_bit_width(2 * this->superstring.size());

        fmt::print(out, "module {}_offset = i{}\n", base_name, offset_bits);
        fmt::print(out, "let {}_table_size: i64 = {}\n", base_name, this->superstring.size());
        fmt::print(out, "let {}_table = [", base_name);

        bool first = true;
        for (auto val : this->superstring) {
            fmt::print(out, "{}{}", first ? first = false, "" : ", ", val);
        }
        fmt::print(out, "] :> [{}_table_size]{}\n", base_name, table_type);

        size_t n_tokens = token_mapping.size();
        auto stringrefs = std::vector<std::vector<String>>(
            n_tokens,
            std::vector<String>(n_tokens, {-1, -1})
        );

        for (const auto& [ap, string] : this->strings) {
            auto i = token_mapping.at(ap.x);
            auto j = token_mapping.at(ap.y);
            stringrefs[i][j] = string;
        }

        fmt::print(out, "let {0}_refs = [\n    ", base_name);
        bool outer_first = true;
        for (const auto& v : stringrefs) {
            if (outer_first)
                outer_first = false;
            else
                fmt::print(out, ",\n    ");

            fmt::print(out, "[");
            bool inner_first = true;
            for (const auto& [offset, size] : v) {
                if (inner_first)
                    inner_first = false;
                else
                    fmt::print(out, ", ");
                fmt::print(out, "({}, {})", offset, size);
            }
            fmt::print(out, "]");
        }

        fmt::print(out, "\n] :> [num_tokens][num_tokens](i{0}, i{0})\n", offset_bits);
    }

    struct Renderer {
        std::ostream& out;
        const Grammar& g;
        const ParsingTable& pt;
        std::unordered_map<Terminal, size_t, Terminal::Hash> token_mapping;
        std::unordered_map<Symbol, size_t, Symbol::Hash> symbol_mapping;

        Renderer(std::ostream& out, const Grammar& g, const ParsingTable& pt);
        size_t bracket_id(const Symbol& sym, bool left) const;
        void render_production_type();
        void render_token_type();
        void render_stack_change_table();
        void render_parse_table();
        void render_production_arities();
    };

    Renderer::Renderer(std::ostream& out, const Grammar& g, const ParsingTable& pt):
        out(out), g(g), pt(pt) {

        for (const auto& [ap, entry] : pt.table) {
            for (const auto& sym : entry.initial_stack)
                this->symbol_mapping.insert({sym, this->symbol_mapping.size()});
            for (const auto& sym : entry.final_stack)
                this->symbol_mapping.insert({sym, this->symbol_mapping.size()});
        }

        for (const auto& prod : g.productions) {
            for (const auto& sym : prod.rhs) {
                if (sym.is_terminal())
                    this->token_mapping.insert({sym.as_terminal(), this->token_mapping.size()});
            }
        }
    }

    size_t Renderer::bracket_id(const Symbol& sym, bool left) const {
        auto id = this->symbol_mapping.at(sym);

        // Left brackets get odd ID's, rightb rackets get even ID's.
        // This way, we can perform a simply subtract and reduce by bit and to
        // check if all the brackets match up.
        return left ? id * 2 + 1 : id * 2;
    }

    void Renderer::render_production_type() {
        auto n = this->g.productions.size();
        auto bits = pareas::int_bit_width(n);
        fmt::print(this->out, "module production = u{}\n", bits);
        fmt::print(this->out, "let num_productions: i64 = {}\n", n);

        // Tags are already guaranteed to be unique, so we don't need to do any kind
        // of deduplication here. As added bonus, the ID of a production is now only
        // dependent on the order in which the productions are defined.
        for (size_t i = 0; i < n; ++i) {
            const auto& prod = this->g.productions[i];
            fmt::print(this->out, "let production_{}: production.t = {}\n", prod.tag, i);
        }
    }

    void Renderer::render_token_type() {
        auto bits = pareas::int_bit_width(this->token_mapping.size());
        fmt::print(this->out, "module token = u{}\n", bits);
        fmt::print(this->out, "let num_tokens: i64 = {}\n", this->token_mapping.size());

        for (const auto& [token, id] : this->token_mapping) {
            switch (token.type) {
                case Terminal::Type::USER_DEFINED:
                    fmt::print(this->out, "let token_{}: token.t = {}\n", token, id);
                    break;
                case Terminal::Type::START_OF_INPUT:
                    fmt::print(this->out, "let special_token_soi: token.t = {}\n", id);
                    break;
                case Terminal::Type::END_OF_INPUT:
                    fmt::print(this->out, "let special_token_eoi: token.t = {}\n", id);
                    break;
                case Terminal::Type::EMPTY:
                    assert(false);
            }
        }
    }

    void Renderer::render_stack_change_table() {
        auto insert_rbr = [&](std::vector<size_t>& result, const ParsingTable::Entry& entry) {
            auto syms = entry.initial_stack;
            for (auto it = syms.rbegin(); it != syms.rend(); ++it) {
                result.push_back(this->bracket_id(*it, false));
            }
        };

        auto insert_lbr = [&](std::vector<size_t>& result, const ParsingTable::Entry& entry) {
            auto syms = entry.final_stack;
            for (auto it = syms.begin(); it != syms.end(); ++it) {
                result.push_back(this->bracket_id(*it, true));
            }
        };

        auto strtab = StringTable<size_t>(
            this->pt,
            [&](const ParsingTable::Entry& entry) {
                auto string = std::vector<size_t>();
                insert_rbr(string, entry);
                insert_lbr(string, entry);
                return string;
            }
        );

        size_t bracket_bits = pareas::int_bit_width(2 * this->symbol_mapping.size());
        fmt::print(this->out, "module bracket = u{}\n", bracket_bits);
        strtab.render(this->out, "stack_change", fmt::format("u{}", bracket_bits), this->token_mapping);
    }

    void Renderer::render_parse_table() {
           auto strtab = StringTable<std::string>(
            this->pt,
            [&](const ParsingTable::Entry& entry) {
                auto result = std::vector<std::string>();
                for (const auto* prod : entry.productions)
                    result.push_back(fmt::format("production_{}", prod->tag));
                return result;
            }
        );
        strtab.render(this->out, "parse", "production.t", this->token_mapping);
    }

    void Renderer::render_production_arities() {
        // Production id's are assigned according to their index in the
        // productions vector, so we can just push_back the arities.
        auto arities = std::vector<size_t>();
        for (const auto& prod : this->g.productions) {
            arities.push_back(prod.arity());
        }

        fmt::print(out, "let production_arity = [{}] :> [num_productions]i32\n", fmt::join(arities, ", "));
    }
}

namespace pareas::parser::llp {
    void render_parser(std::ostream& out, const Grammar& g, const ParsingTable& pt) {
        auto renderer = Renderer(out, g, pt);
        renderer.render_production_type();
        renderer.render_production_arities();
        renderer.render_token_type();
        renderer.render_stack_change_table();
        renderer.render_parse_table();
    }
}
