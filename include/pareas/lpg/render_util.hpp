#ifndef _PAREAS_LPG_RENDER_UTIL_HPP
#define _PAREAS_LPG_RENDER_UTIL_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>

namespace pareas {
    template <typename T>
    constexpr T int_bit_width(T x) {
        auto width = std::max(T{8}, std::bit_ceil(std::bit_width(x)));
        // This should only happen if there are a LOT of rules anyway.
        assert(width <= 64);
        return width;
    }
}

#endif
