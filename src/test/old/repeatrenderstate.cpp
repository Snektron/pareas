#include "test/repeatrenderstate.hpp"
#include "test/coderenderer.hpp"

RepeatRenderState::RepeatRenderState(const RenderState* child, size_t avg, size_t stddev) : child(child), avg(avg), stddev(stddev) {}

void RepeatRenderState::render(CodeRenderer& renderer, std::ostream& os) const {
    std::normal_distribution<> distr(this->avg, this->stddev);
    long long result = std::llround(distr(renderer.get_rng()));
    for(long long i = 0; i < result; ++i) {
        this->child->render(renderer, os);
    }
}