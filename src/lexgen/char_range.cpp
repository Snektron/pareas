#include "pareas/lexgen/char_range.hpp"

#include <cassert>

namespace pareas {
    bool CharRange::contains(unsigned char c) const {
        return this->min <= c && c <= this->max;
    }

    bool CharRange::intersecting_or_adjacent(const CharRange& other) const {
        return this->min <= other.max && this->max + 1 >= other.min;
    }

    void CharRange::merge(const CharRange& other) {
        assert(this->intersecting_or_adjacent(other));
        if (other.min < this->min)
            this->min = other.min;

        if (other.max > this->max)
            this->max = other.max;
    }
}
