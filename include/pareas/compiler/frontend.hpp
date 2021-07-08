#ifndef _PAREAS_COMPILER_FRONTEND_HPP
#define _PAREAS_COMPILER_FRONTEND_HPP

#include "futhark_generated.h"

#include "pareas/compiler/ast.hpp"

#include <chrono>
#include <stdexcept>
#include <iosfwd>

namespace frontend {
    // Keep in sync with src/compiler/frontend.fut
    enum class Status : uint8_t {
        OK = 0,
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

    const char* status_name(Status s);

    struct CompileError: std::runtime_error {
        CompileError(Status status):
            std::runtime_error(status_name(status)) {}
    };

    struct CombinedStatistics {
        std::chrono::microseconds table_upload;
        std::chrono::microseconds input_upload;
        std::chrono::microseconds compile;
        std::chrono::microseconds total;

        void dump(std::ostream& os) const;
    };

    DeviceAst compile_combined(futhark_context* ctx, const std::string& input, CombinedStatistics& stats);

    struct SeparateStatistics {
        std::chrono::microseconds table_upload;
        std::chrono::microseconds input_upload;
        std::chrono::microseconds tokenize;
        std::chrono::microseconds parse;
        std::chrono::microseconds build_parse_tree;
        std::chrono::microseconds fix_bin_ops;
        std::chrono::microseconds fix_if_else;
        std::chrono::microseconds flatten_lists;
        std::chrono::microseconds fix_names;
        std::chrono::microseconds fix_ascriptions;
        std::chrono::microseconds fix_fn_decls;
        std::chrono::microseconds fix_args_and_params;
        std::chrono::microseconds fix_decls;
        std::chrono::microseconds remove_marker_nodes;
        std::chrono::microseconds compute_prev_siblings;
        std::chrono::microseconds check_assignments;
        std::chrono::microseconds insert_derefs;
        std::chrono::microseconds extract_lexemes;
        std::chrono::microseconds resolve_vars;
        std::chrono::microseconds resolve_fns;
        std::chrono::microseconds resolve_args;
        std::chrono::microseconds resolve_data_types;
        std::chrono::microseconds check_return_types;
        std::chrono::microseconds check_convergence;
        std::chrono::microseconds build_ast;
        std::chrono::microseconds total;

        void dump(std::ostream& os) const;
    };

    DeviceAst compile_separate(futhark_context* ctx, const std::string& input, SeparateStatistics& stats);
}

#endif
