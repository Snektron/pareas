#ifndef _PAREAS_TEST_REPEATRENDERSTATE_HPP
#define _PAREAS_TEST_REPEATRENDERSTATE_HPP

#include "test/renderstate.hpp"

#include <random>

class RepeatRenderState : public RenderState {
    private:
        const RenderState* child;
        size_t avg;
        size_t stddev;
    public:
        RepeatRenderState(const RenderState*, size_t, size_t);
        ~RepeatRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
