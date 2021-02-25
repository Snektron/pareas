#ifndef _PAREAS_COMMON_ESCAPE_HPP
#define _PAREAS_COMMON_ESCAPE_HPP

#include <fmt/format.h>

#include <cctype>
#include <cstdlib>

namespace pareas {
    struct EscapeFormatter {
        int c;
    };
}

template <>
struct fmt::formatter<pareas::EscapeFormatter> {
    bool regex_extended = false;

    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        auto end = ctx.end();

        if (it != end && *it == 'r') {
            this->regex_extended = true;
            ++it;
        }

        if (it != end && *it != '}')
            throw format_error("invalid format");

        return it;
    }

    template <typename FormatContext>
    auto format(pareas::EscapeFormatter e, FormatContext& ctx) {
        switch (e.c) {
            case '\\':
                return format_to(ctx.out(), "\\\\");
            case '\n':
                return format_to(ctx.out(), "\\n");
            case '\t':
                return format_to(ctx.out(), "\\t");
            case '\r':
                return format_to(ctx.out(), "\\r");
            case '[':
            case ']':
            case ')':
            case '(':
            case '*':
            case '/':
                if (this->regex_extended)
                    return format_to(ctx.out(), "\\{}", static_cast<char>(e.c));
                // fallthrough
            default:
                if (std::isprint(static_cast<unsigned char>(e.c)))
                    return format_to(ctx.out(), "{}", static_cast<char>(e.c));
                else
                    return format_to(ctx.out(), "\\x{:08X}", e.c);
        }
    }
};

#endif
