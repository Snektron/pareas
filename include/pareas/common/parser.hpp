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
        bool eat_delim(bool eat_newlines = true);

        std::string_view word(); // [a-zA-Z_][a-zA-Z0-9]*

        void skip_until(int end);

        bool is_word_start_char(int c) const;
        bool is_word_continue_char(int c) const;
    };
}

#endif
