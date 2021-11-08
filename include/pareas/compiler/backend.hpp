#ifndef _PAREAS_COMPILER_BACKEND_HPP
#define _PAREAS_COMPILER_BACKEND_HPP

#include "futhark_generated.h"

#include "pareas/compiler/ast.hpp"
#include "pareas/compiler/module.hpp"
#include "pareas/profiler/profiler.hpp"

namespace backend {
    DeviceModule compile(futhark_context* ctx, DeviceAst& ast, pareas::Profiler& p);
}

#endif
