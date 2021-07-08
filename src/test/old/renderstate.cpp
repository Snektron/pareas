#include "test/renderstate.hpp"

#include <stdexcept>

void RenderState::setChild(size_t, const RenderState*) {
    throw std::runtime_error("Invalid renderstate operation");
}