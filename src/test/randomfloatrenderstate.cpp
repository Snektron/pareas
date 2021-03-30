#include "test/randomfloatrenderstate.hpp"
#include "test/coderenderer.hpp"

#include <iostream>

RandomFloatRenderState::RandomFloatRenderState(float min, float max) : min(min), max(max) {}

void RandomFloatRenderState::render(CodeRenderer& renderer, std::ostream&  os) const {
    std::uniform_real_distribution<float> distr(this->min, this->max);

    os << distr(renderer.get_rng());
}