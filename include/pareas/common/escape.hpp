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
    bool escape_regex = false;
    bool escape_quotes = false;

    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        auto end = ctx.end();

        while (it != end && *it != '}') {
            switch (*it++) {
                case 'r':
                    this->escape_regex = true;
                    break;
                case 'q':
                    this->escape_quotes = true;
                    break;
                default:
                    throw format_error("invalid format");
            }
        }

        return it;
    }

    template <typename FormatContext>
    auto format(pareas::EscapeFormatter e, FormatContext& ctx) {
        auto print_escaped_regular = [&] {
            return format_to(ctx.out(), "\\{}", static_cast<char>(e.c));
        };

        switch (e.c) {
                return format_to(ctx.out(), "\\\\");
            case '\n':
                return format_to(ctx.out(), "\\n");
            case '\t':
                return format_to(ctx.out(), "\\t");
            case '\r':
                return format_to(ctx.out(), "\\r");
            case '\'':
            case '\"':
                if (this->escape_quotes)
                    return print_escaped_regular();
                break;
            case '\\':
            case '[':
            case ']':
            case ')':
            case '(':
            case '*':
            case '/':
            case '|':
            case '-':
            case '^':
                if (this->escape_regex)
                    return print_escaped_regular();
                break;
            default:
                break;
        }
        if (std::isprint(static_cast<unsigned char>(e.c)))
            return format_to(ctx.out(), "{}", static_cast<char>(e.c));
        else
            return format_to(ctx.out(), "\\x{:02X}", e.c);
    }
};

#endif
