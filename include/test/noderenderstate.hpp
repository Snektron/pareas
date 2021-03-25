#ifndef _PAREAS_TEST_NODERENDERSTATE_HPP
#define _PAREAS_TEST_NODERENDERSTATE_HPP

#include "test/renderstate.hpp"

#include <vector>

class NodeRenderState : public RenderState {
    private:
        std::vector<const RenderState*> children;
    public:
        NodeRenderState(const std::vector<const RenderState*>&);
        ~NodeRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
