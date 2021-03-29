#ifndef _PAREAS_TEST_IDRENDERSTATE_HPP
#define _PAREAS_TEST_IDRENDERSTATE_HPP

#include "test/renderstate.hpp"

class IDRenderState : public RenderState {
    private:
        const RenderState* fallback;
        size_t category;
    public:
        IDRenderState(const RenderState*, size_t = 0);
        ~IDRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
