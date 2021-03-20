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
    using namespace pareas;
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
            const TokenMapping* tm
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
        const TokenMapping* tm
    ) {
        fmt::print(out, "let {}_table_size: i64 = {}\n", base_name, this->superstring.size());
        fmt::print(out, "let {}_table = [", base_name);

        bool first = true;
        for (auto val : this->superstring) {
            fmt::print(out, "{}{}", first ? first = false, "" : ", ", val);
        }
        fmt::print(out, "] :> [{}_table_size]{}\n", base_name, table_type);

        size_t n_tokens = tm->num_tokens();
        auto stringrefs = std::vector<std::vector<String>>(
            n_tokens,
            std::vector<String>(n_tokens, {-1, -1})
        );

        for (const auto& [ap, string] : this->strings) {
            auto i = tm->token_id(ap.x.as_token());
            auto j = tm->token_id(ap.y.as_token());
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

        fmt::print(out, "\n] :> [num_tokens][num_tokens](i{0}, i{0})\n", Renderer::TABLE_OFFSET_BITS);
    }
}

namespace pareas::parser::llp {
    Renderer::Renderer(const TokenMapping* tm, const Grammar* g, const ParsingTable* pt):
        tm(tm), g(g), pt(pt) {

        for (const auto& [ap, entry] : this->pt->table) {
            for (const auto& sym : entry.initial_stack)
                this->symbol_mapping.insert({sym, this->symbol_mapping.size()});
            for (const auto& sym : entry.final_stack)
                this->symbol_mapping.insert({sym, this->symbol_mapping.size()});
        }
    }

    void Renderer::render_futhark(std::ostream& out) const {
        this->render_productions(out);
        this->render_production_arities(out);
        this->render_stack_change_table(out);
        this->render_parse_table(out);
    }

    size_t Renderer::bracket_id(const Symbol& sym, bool left) const {
        auto id = this->symbol_mapping.at(sym);

        // Left brackets get odd ID's, rightb rackets get even ID's.
        // This way, we can perform a simply subtract and reduce by bit and to
        // check if all the brackets match up.
        return left ? id * 2 + 1 : id * 2;
    }

    void Renderer::render_productions(std::ostream& out) const {
        auto n = this->g->productions.size();
        auto bits = pareas::int_bit_width(n);
        fmt::print(out, "module production = u{}\n", bits);
        fmt::print(out, "let num_productions: i64 = {}\n", n);

        // Tags are already guaranteed to be unique, so we don't need to do any kind
        // of deduplication here. As added bonus, the ID of a production is now only
        // dependent on the order in which the productions are defined.
        for (size_t i = 0; i < n; ++i) {
            const auto& prod = this->g->productions[i];
            fmt::print(out, "let production_{}: production.t = {}\n", prod.tag, i);
        }
    }

    void Renderer::render_production_arities(std::ostream& out) const {
        // Production id's are assigned according to their index in the
        // productions vector, so we can just push_back the arities.
        auto arities = std::vector<size_t>();
        for (const auto& prod : this->g->productions) {
            arities.push_back(prod.arity());
        }

        fmt::print(out, "let production_arity = [{}] :> [num_productions]i32\n", fmt::join(arities, ", "));
    }

    void Renderer::render_stack_change_table(std::ostream& out) const {
        auto strtab = StringTable<size_t>(
            *this->pt,
            [&](const ParsingTable::Entry& entry) {
                auto result = std::vector<size_t>();

                for (auto it = entry.initial_stack.rbegin(); it != entry.initial_stack.rend(); ++it) {
                    result.push_back(this->bracket_id(*it, false));
                }

                for (auto it = entry.final_stack.begin(); it != entry.final_stack.end(); ++it) {
                    result.push_back(this->bracket_id(*it, true));
                }

                return result;
            }
        );

        size_t bracket_bits = pareas::int_bit_width(2 * this->symbol_mapping.size());
        fmt::print(out, "module bracket = u{}\n", bracket_bits);
        strtab.render(out, "stack_change", fmt::format("u{}", bracket_bits), this->tm);
    }

    void Renderer::render_parse_table(std::ostream& out) const {
           auto strtab = StringTable<std::string>(
            *this->pt,
            [&](const ParsingTable::Entry& entry) {
                auto result = std::vector<std::string>();
                for (const auto* prod : entry.productions)
                    result.push_back(fmt::format("production_{}", prod->tag));
                return result;
            }
        );
        strtab.render(out, "parse", "production.t", this->tm);
    }
}
