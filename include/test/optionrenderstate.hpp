#ifndef _PAREAS_TEST_OPTIONRENDERSTATE_HPP
#define _PAREAS_TEST_OPTIONRENDERSTATE_HPP

#include "test/renderstate.hpp"

template <typename T>
class OptionRenderState : public RenderState {
    private:
        const std::vector<const RenderState*> states;
        T callback;
    public:
        OptionRenderState(const std::vector<const RenderState*>, T);
        ~OptionRenderState() = default;

        void render(CodeRenderer&, std::ostream&) const override;
};

template <typename T>
OptionRenderState<T>::OptionRenderState(const std::vector<const RenderState*> states, T callback) : states(states), callback(callback) {}

template <typename T>
void OptionRenderState<T>::render(CodeRenderer& renderer, std::ostream& os) const {
    callback(renderer, this->states)->render(renderer, os);
}

#endif
