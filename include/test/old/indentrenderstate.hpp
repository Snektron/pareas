#ifndef _PAREAS_TEST_INDENTRENDERSTATE_HPP
#define _PAREAS_TEST_INDENTRENDERSTATE_HPP

#include "test/renderstate.hpp"

class IndentRenderState : public RenderState {
    public:
        IndentRenderState();
        ~IndentRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
