#include "pareas/llpgen/llp/render.hpp"
#include "pareas/llpgen/llp/admissible_pair.hpp"

#include <bit>
#include <unordered_map>
#include <algorithm>
#include <ostream>
#include <iterator>
#include <sstream>
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

        void render(std::ostream& out, const std::string& base_var, const std::string& table_type);
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
    void StringTable<T>::render(std::ostream& out, const std::string& base_var, const std::string& table_type) {
        // Add one for sign
        size_t offset_bits = int_bit_width(1 + this->superstring.size());

        out << "let " << base_var << "_table_size: i" << offset_bits << " = " << this->superstring.size() << "\n";
        out << "let stack_change_table: []" << table_type << " = [";
        bool first = true;
        for (auto val : this->superstring) {
            out << (first ? first = false, "" : ", ") << val;
        }
        out << "]\n";
        out << "let initial_stack: (i" << offset_bits << ", i" << offset_bits << ") = " << this->start << "\n";
        out << "let get_stack_change 'terminal (a: terminal) (b: terminal): (i" << offset_bits << ", i" << offset_bits << ") =\n";
        out << "    match (a, b)\n";
        for (const auto& [ap, string] : this->strings) {
            out << "    case (#" << ap.x << ", #" << ap.y << ") -> " << string << "\n";
        }
        out << "    case _ -> (-1, -1)\n";
    }

    struct Renderer {
        std::ostream& out;
        const Grammar& g;
        const ParsingTable& pt;
        std::unordered_map<Symbol, size_t> symbol_mapping;

        Renderer(std::ostream& out, const Grammar& g, const ParsingTable& pt);
        size_t bracket_id(const Symbol& sym, bool left) const;
        void render_production_type();
        void render_stack_change();
        void render_partial_parse();
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
        this->out << "type production = ";
        bool first = true;
        for (const auto& prod : this->g.productions) {
            if (first)
                first = false;
            else
                this->out << " | ";
            this->out << "#" << prod.tag;
        }
        this->out << "\n";
    }

    void Renderer::render_stack_change() {
        auto insert_rbr = [&](std::vector<size_t>& result, const ParsingTable::Entry& entry) {
            auto syms = entry.initial_stack;
            for (auto it = syms.rbegin(); it != syms.rend(); ++it) {
                result.push_back(this->bracket_id(*it, false));
            }
        };

        auto insert_lbr = [&](std::vector<size_t>& result, const ParsingTable::Entry& entry) {
            auto syms = entry.initial_stack;
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

        size_t backing_bits = int_bit_width(1 + this->symbol_mapping.size());
        std::stringstream ss;
        ss << "u" << backing_bits;
        strtab.render(this->out, "stack_change", ss.str());
    }

    void Renderer::render_partial_parse() {
        auto get_tags = [&](const ParsingTable::Entry& entry) {
            auto result = std::vector<Enum>();
            for (const auto* prod : entry.productions)
                result.push_back(Enum{prod->tag});
            return result;
        };

        auto strtab = StringTable<Enum>(this->pt, get_tags, get_tags);

        strtab.render(this->out, "partial_parse", "production");
    }
}

namespace llp {
    void render_parser(std::ostream& out, const Grammar& g, const ParsingTable& pt) {
        auto renderer = Renderer(out, g, pt);
        renderer.render_production_type();
        renderer.render_stack_change();
        renderer.render_partial_parse();
    }
}
