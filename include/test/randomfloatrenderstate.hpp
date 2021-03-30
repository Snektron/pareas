#ifndef _PAREAS_TEST_RANDOMFLOATRENDERSTATE_HPP
#define _PAREAS_TEST_RANDOMFLOATRENDERSTATE_HPP

#include "test/renderstate.hpp"

class RandomFloatRenderState : public RenderState {
    private:
        float min, max;
    public:
        RandomFloatRenderState(float, float);
        ~RandomFloatRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
