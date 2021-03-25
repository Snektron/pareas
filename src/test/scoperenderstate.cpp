#include "test/scoperenderstate.hpp"
#include "test/coderenderer.hpp"

ScopeRenderState::ScopeRenderState(const RenderState* child) : child(child) {}

void ScopeRenderState::render(CodeRenderer& renderer, std::ostream& os) const {
    size_t indent = renderer.getIndent();
    renderer.enterScope();
    renderer.setIndent(indent + 1);

    this->child->render(renderer, os);

    renderer.setIndent(indent);
    renderer.exitScope();
}