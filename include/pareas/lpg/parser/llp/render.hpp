#ifndef _PAREAS_LPG_PARSER_LLP_RENDER_HPP
#define _PAREAS_LPG_PARSER_LLP_RENDER_HPP

#include "pareas/lpg/parser/llp/parsing_table.hpp"
#include "pareas/lpg/parser/grammar.hpp"
#include <iosfwd>

namespace pareas::parser::llp {
    void render_parser(std::ostream& out, const Grammar& g, const ParsingTable& pt);
}

#endif
