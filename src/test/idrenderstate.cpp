#include "test/idrenderstate.hpp"
#include "test/coderenderer.hpp"

#include <iostream>

IDRenderState::IDRenderState(const RenderState* fallback, size_t category) : fallback(fallback), category(category) {}

void IDRenderState::render(CodeRenderer& renderer, std::ostream& os) const {
    bool valid;
    std::string id = renderer.getRandomID(this->category, valid);

    if(!valid) {
        this->fallback->render(renderer, os);
    }
    else {
        os << id;
    }
}