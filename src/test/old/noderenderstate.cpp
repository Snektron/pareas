#include "test/noderenderstate.hpp"
#include "test/coderenderer.hpp"

NodeRenderState::NodeRenderState(const std::vector<const RenderState*>& children) : children(children) {}

void NodeRenderState::render(CodeRenderer& renderer, std::ostream& os) const {
    size_t depth = renderer.getDepth();
    renderer.setDepth(depth + 1);
    for(const RenderState* s : this->children) {
        s->render(renderer, os);
    }
    renderer.setDepth(depth);
}

void NodeRenderState::setChild(size_t idx, const RenderState* child) {
    this->children[idx] = child;
}