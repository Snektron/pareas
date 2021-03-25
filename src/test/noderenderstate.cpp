#include "test/noderenderstate.hpp"

NodeRenderState::NodeRenderState(const std::vector<const RenderState*>& children) : children(children) {}

void NodeRenderState::render(CodeRenderer& renderer, std::ostream& os) const {
    for(const RenderState* s : this->children) {
        s->render(renderer, os);
    }
}