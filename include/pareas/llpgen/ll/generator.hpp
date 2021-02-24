#ifndef _PAREAS_LLPGEN_LL_GENERATOR_HPP
#define _PAREAS_LLPGEN_LL_GENERATOR_HPP

#include "pareas/llpgen/terminal_set_functions.hpp"
#include "pareas/llpgen/error_reporter.hpp"
#include "pareas/llpgen/ll/parsing_table.hpp"

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
