#include "pareas/llpgen/llp/render.hpp"
#include "pareas/llpgen/llp/admissible_pair.hpp"

#include <fmt/ostream.h>

#include <bit>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <cassert>

namespace {
    using llp::AdmissiblePair;
    using llp::ParsingTable;

    size_t int_bit_width(size_t x) {
        auto width = std::max(size_t{8}, std::bit_ceil(std::bit_width(x)));
        // This should only happen if there are a LOT of rules anyway.
        assert(width <= 64);
        return width;
    }

    struct String {
        size_t offset;
        size_t size;
    };

    std::ostream& operator<<(std::ostream& os, const String& str) {
        return os << "(" << str.offset << ", " << str.size << ")";
    }

    struct Enum {
        std::string name;
    };

    std::ostream& operator<<(std::ostream& os, const Enum& st) {
        return os << "#" << st.name;
    }

    template <typename T>
    struct StringTable {
        std::vector<T> superstring;
        String start;
        std::unordered_map<AdmissiblePair, String> strings;

        template <typename F, typename G>
        StringTable(const ParsingTable& pt, F get_start_string, G get_string);

        void render(std::ostream& out, const std::string& base_name, const std::string& table_type);
    };

    template <typename T>
    template <typename F, typename G>
    StringTable<T>::StringTable(const ParsingTable& pt, F get_start_string, G get_string) {
        // Simple implementation for now
        this->superstring = get_start_string(pt.start);
        auto offset = this->superstring.size();
        this->start = {0, offset};

        for (const auto& [ap, entry] : pt.table) {
            auto string = get_string(entry);
            this->superstring.insert(superstring.end(), string.begin(), string.end());
            this->strings[ap] = {offset, string.size()};
            offset += string.size();
        }
    }

    template <typename T>
    void StringTable<T>::render(std::ostream& out, const std::string& base_name, const std::string& table_type) {
        // Multiply by 2 to account for the sign bit
        size_t offset_bits = int_bit_width(2 * this->superstring.size());

        fmt::print(out, "module {}_offset = i{}\n", base_name, offset_bits);
        fmt::print(out, "let {}_table_size: i64 = {}\n", base_name, this->superstring.size());
        fmt::print(out, "let {}_table = [", base_name);

        bool first = true;
        for (auto val : this->superstring) {
            fmt::print(out, "{}{}", first ? first = false, "" : ", ", val);
        }
        fmt::print(out, "] :> [{}_table_size]{}\n", base_name, table_type);

        fmt::print(out, "let initial_{0}: (i{1}, i{1}) = {2}\n", base_name, offset_bits, this->start);
        fmt::print(out, "let get_{0} (a: token) (b: token): (i{1}, i{1}) =\n", base_name, offset_bits);

        fmt::print(out, "    match (a, b)\n");
        for (const auto& [ap, string] : this->strings) {
            fmt::print(out, "    case (#{}, #{}) -> {}\n", ap.x, ap.y, string);
        }
        fmt::print(out, "    case _ -> (-1, -1)\n");
    }

    struct Renderer {
        std::ostream& out;
        const Grammar& g;
        const ParsingTable& pt;
        std::unordered_map<Symbol, size_t> symbol_mapping;

        Renderer(std::ostream& out, const Grammar& g, const ParsingTable& pt);
        size_t bracket_id(const Symbol& sym, bool left) const;
        void render_production_type();
        void render_token_type();
        void render_stack_change_table();
        void render_parse_table();
    };

    Renderer::Renderer(std::ostream& out, const Grammar& g, const ParsingTable& pt):
        out(out), g(g), pt(pt) {

        auto add_sym = [&](const Symbol& sym) {
            symbol_mapping.insert({sym, symbol_mapping.size()});
        };

        auto add_entry = [&](const llp::ParsingTable::Entry& entry) {
            for (const auto& sym : entry.initial_stack)
                add_sym(sym);
            for (const auto& sym : entry.final_stack)
                add_sym(sym);
        };

        add_entry(pt.start);
        for (const auto& [ap, entry] : pt.table) {
            add_entry(entry);
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
        fmt::print(this->out, "type production = ");
        bool first = true;
        for (const auto& prod : this->g.productions) {
            if (first)
                first = false;
            else
                fmt::print(this->out, " | ");
            fmt::print(this->out, "#{}", prod.tag);
        }
        fmt::print(this->out, "\n");
    }

    void Renderer::render_token_type() {
        auto all_terminals = std::unordered_set<std::string>();

        for (const auto& prod : this->g.productions) {
            for (const auto& sym : prod.rhs) {
                if (sym.is_terminal)
                    all_terminals.insert(sym.name);
            }
        }

        fmt::print(this->out, "type token = ");
        bool first = true;
        for (const auto& term : all_terminals) {
            if (first)
                first = false;
            else
                fmt::print(this->out, " | ");
            fmt::print(this->out, "#{}", term);
        }
        fmt::print(this->out, "\n");
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
                insert_lbr(string, entry);
                return string;
            },
            [&](const ParsingTable::Entry& entry) {
                auto string = std::vector<size_t>();
                insert_rbr(string, entry);
                insert_lbr(string, entry);
                return string;
            }
        );

        size_t bracket_bits = int_bit_width(2 * this->symbol_mapping.size());
        fmt::print(this->out, "module bracket = u{}\n", bracket_bits);
        strtab.render(this->out, "stack_change", fmt::format("u{}", bracket_bits));
    }

    void Renderer::render_parse_table() {
        auto get_tags = [&](const ParsingTable::Entry& entry) {
            auto result = std::vector<Enum>();
            for (const auto* prod : entry.productions)
                result.push_back(Enum{prod->tag});
            return result;
        };

        auto strtab = StringTable<Enum>(this->pt, get_tags, get_tags);
        strtab.render(this->out, "parse", "production");
    }
}

namespace llp {
    void render_parser(std::ostream& out, const Grammar& g, const ParsingTable& pt) {
        auto renderer = Renderer(out, g, pt);
        renderer.render_production_type();
        renderer.render_token_type();
        renderer.render_stack_change_table();
        renderer.render_parse_table();
    }
}
