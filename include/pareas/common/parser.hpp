#ifndef _PAREAS_COMMON_PARSER_HPP
#define _PAREAS_COMMON_PARSER_HPP

#include "pareas/common/error_reporter.hpp"

#include <string_view>
#include <cstddef>

namespace pareas {
    struct Parser {
        ErrorReporter* er;
        std::string_view source;
        size_t offset;

        Parser(ErrorReporter* er, std::string_view source);

        SourceLocation loc() const;

        int peek() const;
        int consume();
        bool eat(int c);
        bool expect(int c);
    };
}

#endif
