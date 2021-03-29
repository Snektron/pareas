#include "test/randomintrenderstate.hpp"
#include "test/coderenderer.hpp"

#include <iostream>

RandomIntRenderState::RandomIntRenderState(size_t min, size_t max) : min(min), max(max) {}

void RandomIntRenderState::render(CodeRenderer& renderer, std::ostream&  os) const {
    std::uniform_int_distribution<size_t> distr(this->min, this->max);

    os << distr(renderer.get_rng());
}