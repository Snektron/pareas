#ifndef _PAREAS_LLPGEN_HASH_UTIL_HPP
#define _PAREAS_LLPGEN_HASH_UTIL_HPP

#include <functional>
#include <cstddef>
#include <cassert>

constexpr size_t hash_combine(size_t lhs, size_t rhs) {
    // Ye olde boost hash combine
    return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
}

template <typename It, typename Hasher>
size_t hash_iterator_range(It begin, It end, Hasher hasher) {
    size_t hash = 0;
    while (begin != end) {
        hash = hash_combine(hash, hasher(*begin++));
    }
    return hash;
}

#endif
