#include "test/idcreaterenderstate.hpp"
#include "test/coderenderer.hpp"

#include <sstream>
#include <iostream>

const char ID_CHARS[] = {
    '_',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F',
    'G',
    'H',
    'I',
    'J',
    'K',
    'L',
    'M',
    'N',
    'O',
    'P',
    'Q',
    'R',
    'S',
    'T',
    'U',
    'V',
    'W',
    'X',
    'Y',
    'Z',
    'a',
    'b',
    'c',
    'd',
    'e',
    'f',
    'g',
    'h',
    'i',
    'j',
    'k',
    'l',
    'm',
    'n',
    'o',
    'p',
    'q',
    'r',
    's',
    't',
    'u',
    'v',
    'w',
    'x',
    'y',
    'z',
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9'
};

IDCreateRenderState::IDCreateRenderState(size_t avg_len, size_t stddev_len) : avg_len(avg_len), stddev_len(stddev_len) {}

void IDCreateRenderState::render(CodeRenderer& renderer, std::ostream& os) const {
    std::string id;
    do {
        std::normal_distribution<> distr(this->avg_len, this->stddev_len);
        long long result = std::llround(distr(renderer.get_rng()));
        if(result < 1)
            result = 1;

        std::uniform_int_distribution<size_t> first_rng(0, 26*2);
        std::stringstream ss;
        ss << ID_CHARS[first_rng(renderer.get_rng())];

        std::uniform_int_distribution<size_t> id_rng(0, 26*2+10);
        for(long long i = 1; i < result; ++i) {
            ss << ID_CHARS[id_rng(renderer.get_rng())];
        }

        id = ss.str();
    } while(!renderer.addId(id));

    os << id;
}