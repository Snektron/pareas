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

    struct StrTab {
        size_t item_bytes;
        std::vector<uint64_t> superstring;
        std::unordered_map<AdmissiblePair, String, AdmissiblePair::Hash> strings;

        template <typename F>
        StrTab(const ParsingTable& pt, size_t item_bytes, F get_string);

        void render(Renderer* r, const TokenMapping* tm, std::string_view name, std::string_view type);
    };

    template <typename F>
    StrTab::StrTab(const ParsingTable& pt, size_t item_bytes, F get_string):
        item_bytes(item_bytes) {
        // Simple implementation for now
        int32_t offset = 0;
        for (const auto& [ap, entry] : pt.table) {
            auto string = get_string(entry);
            this->superstring.insert(superstring.end(), string.begin(), string.end());
            this->strings[ap] = {offset, static_cast<int32_t>(string.size())};
            offset += string.size();
        }
    }

    void StrTab::render(Renderer* r, const TokenMapping* tm, std::string_view name, std::string_view type) {
        size_t n = tm->num_tokens();
        auto stringrefs = std::vector<std::vector<String>>(
            n,
            std::vector<String>(n, {-1, -1})
        );

        for (const auto& [ap, string] : this->strings) {
            auto i = tm->token_id(ap.x.as_token());
            auto j = tm->token_id(ap.y.as_token());
            stringrefs[i][j] = string;
        }

        r->align_data(this->item_bytes);
        auto table_offset = r->data_offset();

        for (auto value : this->superstring) {
            r->write_data_int(value, this->item_bytes);
        }

        r->align_data(sizeof(int32_t));
        auto offsets_offset = r->data_offset();

        for (const auto& v : stringrefs) {
            for (auto str : v) {
                // According to cppreference, this cast is valid and will produce the desired result.
                r->write_data_int(static_cast<uint32_t>(str.offset), sizeof(uint32_t));
            }
        }

        auto lengths_offset = r->data_offset();

        for (const auto& v : stringrefs) {
            for (auto str : v) {
                r->write_data_int(static_cast<uint32_t>(str.size), sizeof(uint32_t));
            }
        }

        fmt::print(r->hpp, "extern const StrTab<{}> {};\n", type, name);

        fmt::print(
            r->cpp,
            "const StrTab<{}> {}= {{\n"
            "    .n = {},\n"
            "    .table = {},\n"
            "    .offsets = {},\n"
            "    .length = {},\n"
            "}};\n",
            type,
            name,
            this->superstring.size(),
            r->render_offset_cast(table_offset, type),
            r->render_offset_cast(offsets_offset, "int32_t"),
            r->render_offset_cast(lengths_offset, "int32_t")
        );
    }
}

namespace pareas::parser::llp {
    ParserRenderer::ParserRenderer(Renderer* r, const TokenMapping* tm, const Grammar* g, const ParsingTable* pt):
        r(r), tm(tm), g(g), pt(pt) {

        for (const auto& [ap, entry] : this->pt->table) {
            for (const auto& sym : entry.initial_stack)
                this->symbol_mapping.insert({sym, this->symbol_mapping.size()});
            for (const auto& sym : entry.final_stack)
                this->symbol_mapping.insert({sym, this->symbol_mapping.size()});
        }
    }

    void ParserRenderer::render() const {
        this->render_productions();

        fmt::print(
            this->r->hpp,
            "template <typename T>\n"
            "struct StrTab {{\n"
            "    size_t n;\n"
            "    const T* table; // n\n"
            "    const uint32_t* offsets; // NUM_TOKENS\n"
            "    const uint32_t* lengths; // NUM_TOKENS\n"
            "}};\n",
            this->bracket_backing_bits()
        );

        this->render_production_arity_data();
        this->render_stack_change_table();
        this->render_parse_table();
    }

    uint64_t ParserRenderer::bracket_id(const Symbol& sym, bool left) const {
        auto id = this->symbol_mapping.at(sym);

        // Left brackets get odd ID's, rightb rackets get even ID's.
        // This way, we can perform a simply subtract and reduce by bit and to
        // check if all the brackets match up.
        return left ? id * 2 + 1 : id * 2;
    }

    size_t ParserRenderer::bracket_backing_bits() const {
        return pareas::int_bit_width(2 * this->symbol_mapping.size());
    }

    void ParserRenderer::render_productions() const {
        auto n = this->g->productions.size();
        auto backing_bits = this->g->production_backing_type_bits();

        fmt::print(this->r->fut, "module production = u{}\n", backing_bits);

        fmt::print(this->r->hpp, "enum class Production : uint{}_t {{\n", backing_bits);

        fmt::print(this->r->cpp, "const char* production_name(Production p) {{\n");
        fmt::print(this->r->cpp, "    switch (p) {{\n");

        // Tags are already guaranteed to be unique, so we don't need to do any kind
        // of deduplication here. As added bonus, the ID of a production is now only
        // dependent on the order in which the productions are defined.
        for (size_t id = 0; id < n; ++id) {
            const auto& tag = this->g->productions[id].tag;

            auto tag_upper = tag;
            std::transform(tag_upper.begin(), tag_upper.end(), tag_upper.begin(), ::toupper);

            fmt::print(this->r->fut, "let production_{}: production.t = {}\n", tag, id);

            fmt::print(this->r->hpp, "    {} = {},\n", tag_upper, id);

            fmt::print(this->r->cpp, "        case Production::{}: return \"{}\";\n", tag_upper, tag);
        }

        fmt::print(this->r->fut, "let num_productions: i64 = {}\n", n);

        fmt::print(this->r->hpp, "}};\n");
        fmt::print(this->r->hpp, "constexpr const size_t NUM_PRODUCTIONS = {};\n", n);
        fmt::print(this->r->hpp, "const char* production_name(Production p);\n");

        fmt::print(this->r->cpp, "    }}\n}}\n");
    }

    void ParserRenderer::render_production_arity_data() const {
        this->r->align_data(sizeof(uint32_t));
        auto offset = this->r->data_offset();

        fmt::print(this->r->hpp, "extern const int32_t* arities; // NUM_PRODUCTIONS\n");

        fmt::print(this->r->cpp, "const int32_t* arities = {};\n", this->r->render_offset_cast(offset, "int32_t"));

        // Production id's are assigned according to their index in the
        // productions vector, so we can just write them in order of definition.
        for (const auto& prod : this->g->productions) {
            this->r->write_data_int(prod.arity(), sizeof(uint32_t));
        }
    }

    void ParserRenderer::render_stack_change_table() const {
        size_t bracket_bits = this->bracket_backing_bits();

        auto strtab = StrTab(
            *this->pt,
            bracket_bits / 8,
            [&](const ParsingTable::Entry& entry) {
                auto result = std::vector<uint64_t>();

                for (auto it = entry.initial_stack.rbegin(); it != entry.initial_stack.rend(); ++it) {
                    result.push_back(this->bracket_id(*it, false));
                }

                for (auto it = entry.final_stack.begin(); it != entry.final_stack.end(); ++it) {
                    result.push_back(this->bracket_id(*it, true));
                }

                return result;
            }
        );

        fmt::print(this->r->hpp, "using Bracket = uint{}_t;\n", bracket_bits);
        strtab.render(this->r, this->tm, "stack_change_table", "Bracket");
    }

    void ParserRenderer::render_parse_table() const {
        size_t backing_bits = this->g->production_backing_type_bits();

        auto strtab = StrTab(
            *this->pt,
            backing_bits / 8,
            [&](const ParsingTable::Entry& entry) {
                auto result = std::vector<uint64_t>();

                for (const auto* prod : entry.productions)
                    result.push_back(this->g->production_id(prod));
                return result;
            }
        );

        strtab.render(this->r, this->tm, "parse_table", "Production");
    }
}
