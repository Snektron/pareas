#ifndef _PAREAS_LEXGEN_CHAR_RANGE_HPP
#define _PAREAS_LEXGEN_CHAR_RANGE_HPP

namespace pareas {
    struct CharRange {
        unsigned char min;
        unsigned char max;

        bool contains(unsigned char c) const;
        bool intersecting_or_adjacent(const CharRange& other) const;
        void merge(const CharRange& other);
    };
}

#endif
