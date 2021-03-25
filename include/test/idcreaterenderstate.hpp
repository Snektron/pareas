#ifndef _PAREAS_TEST_IDCREATERENDERSTATE_HPP
#define _PAREAS_TEST_IDCREATERENDERSTATE_HPP

#include "test/renderstate.hpp"

class IDCreateRenderState : public RenderState {
    private:
        size_t avg_len;
        size_t stddev_len;
    public:
        IDCreateRenderState(size_t, size_t);
        ~IDCreateRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

#endif
