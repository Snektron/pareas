#ifndef _PAREAS_LPG_PARSER_LLP_RENDER_HPP
#define _PAREAS_LPG_PARSER_LLP_RENDER_HPP

#include "pareas/lpg/renderer.hpp"
#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/parser/grammar.hpp"
#include "pareas/lpg/parser/llp/parsing_table.hpp"

#include <iosfwd>
#include <unordered_map>
#include <cstddef>

namespace pareas::parser::llp {
    class ParserRenderer {
        Renderer* r;
        const TokenMapping* tm;
        const Grammar* g;
        const ParsingTable* pt;

        std::unordered_map<Symbol, size_t, Symbol::Hash> symbol_mapping;

    public:
        constexpr const static size_t TABLE_OFFSET_BITS = 32;

        ParserRenderer(Renderer* r, const TokenMapping* tm, const Grammar* g, const ParsingTable* pt);
        void render() const;

    private:
        size_t bracket_id(const Symbol& sym, bool left) const;

        void render_productions() const;
        void render_production_arities() const;
        void render_stack_change_table() const;
        void render_parse_table() const;
    };
}

#endif
