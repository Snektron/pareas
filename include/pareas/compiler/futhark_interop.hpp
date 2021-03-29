#ifndef _PAREAS_COMPILER_FUTHARK_INTEROP_HPP
#define _PAREAS_COMPILER_FUTHARK_INTEROP_HPP

#include "futhark_generated.h"

#include <memory>
#include <string>
#include <stdexcept>
#include <cstdio>

namespace futhark {
    template <typename T, void(*deleter)(T*)>
    struct Deleter {
        void operator()(T* t) const {
            deleter(t);
        }
    };

    template <typename T, void(*deleter)(T*)>
    using Unique = std::unique_ptr<T, Deleter<T, deleter>>;

    using ContextConfig = Unique<futhark_context_config, futhark_context_config_free>;
    using Context = Unique<futhark_context, futhark_context_free>;
}

#endif
