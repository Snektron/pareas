#ifndef _PAREAS_LPG_PARSER_LLP_RENDER_HPP
#define _PAREAS_LPG_PARSER_LLP_RENDER_HPP

#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/parser/grammar.hpp"
#include "pareas/lpg/parser/llp/parsing_table.hpp"
#include <iosfwd>

namespace pareas::parser::llp {
    void render_parser(std::ostream& out, const TokenMapping& tm, const Grammar& g, const ParsingTable& pt);
}

#endif
