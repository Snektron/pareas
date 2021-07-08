#ifndef _PAREAS_TEST_COMMITRENDERSTATE_HPP
#define _PAREAS_TEST_COMMITRENDERSTATE_HPP

#include "test/renderstate.hpp"

class CommitRenderState : public RenderState {
    public:
        ~CommitRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
