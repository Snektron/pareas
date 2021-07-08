#ifndef _PAREAS_TEST_SCOPERENDERSTATE_HPP
#define _PAREAS_TEST_SCOPERENDERSTATE_HPP

#include "test/renderstate.hpp"

class ScopeRenderState : public RenderState {
    private:
        const RenderState* child;
    public:
        ScopeRenderState(const RenderState*);
        ~ScopeRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
