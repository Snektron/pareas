#ifndef _PAREAS_TEST_RANDOMINTRENDERSTATE_HPP
#define _PAREAS_TEST_RANDOMINTRENDERSTATE_HPP

#include "test/renderstate.hpp"

class RandomIntRenderState : public RenderState {
    private:
        size_t min, max;
    public:
        RandomIntRenderState(size_t, size_t);
        ~RandomIntRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
