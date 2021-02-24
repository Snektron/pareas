#ifndef _PAREAS_LLPGEN_LLP_RENDER_HPP
#define _PAREAS_LLPGEN_LLP_RENDER_HPP

#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/llp/parsing_table.hpp"
#include <iosfwd>

namespace pareas::llp {
    void render_parser(std::ostream& out, const Grammar& g, const ParsingTable& pt);
}

#endif
