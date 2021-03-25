#ifndef _PAREAS_TEST_TEXTRENDERSTATE_HPP
#define _PAREAS_TEST_TEXTRENDERSTATE_HPP

#include "test/renderstate.hpp"

#include <string>

class TextRenderState : public RenderState {
    private:
        std::string str;
    public:
        TextRenderState(const std::string&);
        ~TextRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
