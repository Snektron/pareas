#ifndef _PAREAS_LPG_PARSER_LL_GENERATOR_HPP
#define _PAREAS_LPG_PARSER_LL_GENERATOR_HPP

#include "pareas/lpg/parser/terminal_set_functions.hpp"
#include "pareas/lpg/parser/ll/parsing_table.hpp"
#include "pareas/lpg/error_reporter.hpp"

namespace pareas::ll {
    class Generator {
        ErrorReporter* er;
        const Grammar* g;
        const TerminalSetFunctions* tsf;

    public:
        Generator(ErrorReporter* er, const Grammar* g, const TerminalSetFunctions* tsf);
        ParsingTable build_parsing_table();
    };
}

#endif
