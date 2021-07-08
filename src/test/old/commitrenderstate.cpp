#include "test/commitrenderstate.hpp"
#include "test/coderenderer.hpp"

void CommitRenderState::render(CodeRenderer& render, std::ostream& os) const {
    render.commitIds();
}