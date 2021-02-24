#ifndef _PAREAS_COMMON_ERROR_REPORTER_HPP
#define _PAREAS_COMMON_ERROR_REPORTER_HPP

#include <string_view>
#include <vector>
#include <sstream>
#include <ostream>
#include <cstdint>

namespace pareas {
    struct SourceLocation {
        size_t offset;
    };

    struct ErrorReporter {
        std::string_view source;
        std::ostream& out;
        std::vector<size_t> newlines;
        size_t count;

        ErrorReporter(std::string_view source, std::ostream& out);

        void error(SourceLocation loc, std::string_view msg) const;
        void note(SourceLocation loc, std::string_view msg) const;

    private:
        void print(SourceLocation loc, std::string_view tag, std::string_view msg) const;

        struct LineInfo {
            size_t line; // one-indexed
            size_t column; // one-indexed
            std::string_view text;
        };

        LineInfo line(SourceLocation loc) const;
    };
}

#endif
