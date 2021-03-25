#include "test/indentrenderstate.hpp"
#include "test/coderenderer.hpp"

#include <iostream>

IndentRenderState::IndentRenderState() {}

void IndentRenderState::render(CodeRenderer& renderer, std::ostream& os) const {
    for(size_t i = 0; i < renderer.getIndent(); ++i)
        os << "    ";
}