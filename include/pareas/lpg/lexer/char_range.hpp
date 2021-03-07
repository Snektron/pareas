#ifndef _PAREAS_LPG_LEXER_CHAR_RANGE_HPP
#define _PAREAS_LPG_LEXER_CHAR_RANGE_HPP

#include <cstdint>

namespace pareas {
    struct CharRange {
        uint8_t min;
        uint8_t max;

        bool contains(uint8_t c) const;
        bool intersecting_or_adjacent(const CharRange& other) const;
        void merge(const CharRange& other);
    };
}

#endif
