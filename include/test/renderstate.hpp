#ifndef _PAREAS_TEST_RENDERSTATE_HPP
#define _PAREAS_TEST_RENDERSTATE_HPP

#include <iosfwd>

class CodeRenderer;

class RenderState {
    public:
        virtual ~RenderState() = default;
        virtual void render(CodeRenderer&, std::ostream&) const = 0;
};

#endif
