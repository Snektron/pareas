#ifndef _PAREAS_LPG_ESCAPE_HPP
#define _PAREAS_LPG_ESCAPE_HPP

#include <fmt/format.h>

#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <climits>

namespace pareas {
    struct EscapeFormatter {
        uint8_t c;
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
            case '+':
                if (this->escape_regex)
                    return print_escaped_regular();
                break;
            default:
                break;
        }
        if (std::isprint(e.c) && e.c < CHAR_MAX)
            return format_to(ctx.out(), "{}", static_cast<char>(e.c));
        else
            return format_to(ctx.out(), "\\x{:02X}", e.c);
    }
};

#endif
