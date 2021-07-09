#ifndef _PAREAS_COMPILER_FRONTEND_HPP
#define _PAREAS_COMPILER_FRONTEND_HPP

#include "futhark_generated.h"

#include "pareas/compiler/ast.hpp"
#include "pareas/compiler/profiler.hpp"

#include <chrono>
#include <stdexcept>
#include <iosfwd>

namespace frontend {
    enum class Error : uint8_t {
        PARSE_ERROR = 1,
        STRAY_ELSE_ERROR = 2,
        INVALID_DECL = 3,
        INVALID_PARAMS = 4,
        INVALID_ASSIGN = 5,
        INVALID_FN_PROTO = 6,
        DUPLICATE_FN_OR_INVALID_CALL = 7,
        INVALID_VARIABLE = 8,
        INVALID_ARG_COUNT = 9,
        TYPE_ERROR = 10,
        INVALID_RETURN = 11,
        MISSING_RETURN = 12,
    };

    const char* error_name(Error e);

    struct CompileError: std::runtime_error {
        CompileError(Error e):
            std::runtime_error(error_name(e)) {}
    };

    DeviceAst compile(futhark_context* ctx, const std::string& input, Profiler& p);
}

#endif
