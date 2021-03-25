#include "test/textrenderstate.hpp"

TextRenderState::TextRenderState(const std::string& str) : str(str) {}

void TextRenderState::render(CodeRenderer& renderer, std::ostream& output) const {
    output << this->str;
}