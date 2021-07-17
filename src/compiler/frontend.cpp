#include "pareas/compiler/frontend.hpp"
#include "pareas/compiler/futhark_interop.hpp"

#include "pareas_grammar.hpp"

#include <fmt/ostream.h>
#include <fmt/chrono.h>

namespace {
    futhark::UniqueLexTable upload_lex_table(futhark_context* ctx) {
        auto initial_state = futhark::UniqueArray<uint16_t, 1>(
            ctx,
            reinterpret_cast<const grammar::LexTable::State*>(grammar::lex_table.initial_states),
            grammar::LexTable::NUM_INITIAL_STATES
        );

        auto merge_table = futhark::UniqueArray<uint16_t, 2>(
            ctx,
            reinterpret_cast<const grammar::LexTable::State*>(grammar::lex_table.merge_table),
            grammar::lex_table.n,
            grammar::lex_table.n
        );

        auto final_state = futhark::UniqueArray<uint8_t, 1>(
            ctx,
            reinterpret_cast<const std::underlying_type_t<grammar::Token>*>(grammar::lex_table.final_states),
            grammar::lex_table.n
        );

        auto lex_table = futhark::UniqueLexTable(ctx);

        int err = futhark_entry_mk_lex_table(
            ctx,
            &lex_table,
            initial_state.get(),
            merge_table.get(),
            final_state.get()
        );

        if (err)
            throw futhark::Error(ctx);

        return lex_table;
    }

    template <typename T, typename U, typename F>
    T upload_strtab(futhark_context* ctx, const grammar::StrTab<U>& strtab, F upload_fn) {
        static_assert(sizeof(U) == sizeof(uint8_t));

        auto table = futhark::UniqueArray<uint8_t, 1>(
            ctx,
            reinterpret_cast<const uint8_t*>(strtab.table),
            strtab.n
        );

        auto offsets = futhark::UniqueArray<int32_t, 2>(ctx, strtab.offsets, grammar::NUM_TOKENS, grammar::NUM_TOKENS);
        auto lengths = futhark::UniqueArray<int32_t, 2>(ctx, strtab.lengths, grammar::NUM_TOKENS, grammar::NUM_TOKENS);

        auto tab = T(ctx);

        int err = upload_fn(ctx, &tab, table.get(), offsets.get(), lengths.get());
        if (err)
            throw futhark::Error(ctx);

        return tab;
    }
}

namespace frontend {
    const char* error_name(Error e) {
        switch (e) {
            case Error::PARSE_ERROR: return "Parse error";
            case Error::STRAY_ELSE_ERROR: return "Stray else/elif";
            case Error::INVALID_DECL: return "Declaration cannot be both function and variable";
            case Error::INVALID_PARAMS: return "Invalid function parameter list";
            case Error::INVALID_ASSIGN: return "Invalid assignment lvalue";
            case Error::INVALID_FN_PROTO: return "Invalid function prototype";
            case Error::DUPLICATE_FN_OR_INVALID_CALL: return "Duplicate function declaration or call to undefined function";
            case Error::INVALID_VARIABLE: return "Undeclared variable";
            case Error::INVALID_ARG_COUNT: return "Invalid amount of arguments for function call";
            case Error::TYPE_ERROR: return "Type error";
            case Error::INVALID_RETURN: return "Return expression has invalid type";
            case Error::MISSING_RETURN: return "Not all code paths in non-void function return a value";
        }
    }

    DeviceAst compile(futhark_context* ctx, const std::string& input, pareas::Profiler& p, std::FILE* debug_log) {
        auto debug_log_region = [&](const char* name) {
            if (debug_log)
                fmt::print(debug_log, "<<<{}>>>\n", name);
        };

        debug_log_region("upload");
        p.begin();
        p.begin();
        auto lex_table = upload_lex_table(ctx);
        auto sct = upload_strtab<futhark::UniqueStackChangeTable>(
            ctx,
            grammar::stack_change_table,
            futhark_entry_mk_stack_change_table
        );

        auto pt = upload_strtab<futhark::UniqueParseTable>(
            ctx,
            grammar::parse_table,
            futhark_entry_mk_parse_table
        );

        auto arity_array = futhark::UniqueArray<int32_t, 1>(ctx, grammar::arities, grammar::NUM_PRODUCTIONS);
        p.end("table");
        p.begin();

        auto input_array = futhark::UniqueArray<uint8_t, 1>(ctx, reinterpret_cast<const uint8_t*>(input.data()), input.size());
        p.end("input");
        p.end("upload");

        p.begin();

        debug_log_region("tokenize");
        auto tokens = futhark::UniqueTokenArray(ctx);
        p.measure("tokenize", [&]{
            int err = futhark_entry_frontend_tokenize(ctx, &tokens, input_array, lex_table);
            if (err)
                throw futhark::Error(ctx);
        });
        lex_table.clear();

        debug_log_region("parse");
        auto node_types = futhark::UniqueArray<uint8_t, 1>(ctx);
        p.measure("parse", [&]{
            bool valid = false;
            int err = futhark_entry_frontend_parse(ctx, &valid, &node_types, tokens, sct, pt);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::PARSE_ERROR);
        });
        sct.clear();
        pt.clear();

        debug_log_region("build parse tree");
        auto parents = futhark::UniqueArray<int32_t, 1>(ctx);
        p.measure("build parse tree", [&]{
            int err = futhark_entry_frontend_build_parse_tree(ctx, &parents, node_types, arity_array);
            if (err)
                throw futhark::Error(ctx);
        });
        arity_array.clear();

        p.begin();
        debug_log_region("syntax");
        p.measure("fix bin ops", [&]{
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_fix_bin_ops(ctx, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
        });

        p.measure("fix conditionals", [&]{
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            bool valid;
            int err = futhark_entry_frontend_fix_if_else(ctx, &valid, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::STRAY_ELSE_ERROR);
        });

        p.measure("flatten lists", [&]{
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_flatten_lists(ctx, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
        });

        p.measure("fix names", [&]{
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            bool valid;
            int err = futhark_entry_frontend_fix_names(ctx, &valid, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::INVALID_DECL);
        });

        p.measure("fix ascriptions", [&]{
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_fix_ascriptions(ctx, &parents, node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
        });

        p.measure("fix fn decls", [&]{
            auto old_parents = std::move(parents);
            bool valid;
            int err = futhark_entry_frontend_fix_fn_decls(ctx, &valid, &parents, node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::INVALID_FN_PROTO);
        });

        p.measure("fix args and params", [&]{
            auto old_node_types = std::move(node_types);
            int err = futhark_entry_frontend_fix_args_and_params(ctx, &node_types, old_node_types, parents);
            if (err)
                throw futhark::Error(ctx);
        });

        p.measure("fix decls", [&]{
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            bool valid;
            int err = futhark_entry_frontend_fix_decls(ctx, &valid, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::INVALID_DECL);
        });

        p.measure("remove marker nodes", [&]{
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_remove_marker_nodes(ctx, &parents, node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
        });

        auto prev_siblings = futhark::UniqueArray<int32_t, 1>(ctx);
        p.measure("compute prev siblings", [&]{
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_compute_prev_sibling(ctx, &node_types, &parents, &prev_siblings, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
        });

        p.measure("check assignments", [&]{
            bool valid;
            int err = futhark_entry_frontend_check_assignments(ctx, &valid, node_types, parents, prev_siblings);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::INVALID_ASSIGN);
        });
        p.end("syntax");

        p.begin();
        debug_log_region("sema");
        p.measure("insert derefs", [&]{
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            auto old_prev_siblings = std::move(prev_siblings);
            int err = futhark_entry_frontend_insert_derefs(ctx, &node_types, &parents, &prev_siblings, old_node_types, old_parents, old_prev_siblings);
            if (err)
                throw futhark::Error(ctx);
        });

        auto node_data = futhark::UniqueArray<uint32_t, 1>(ctx);
        p.measure("extract lexemes", [&]{
            int err = futhark_entry_frontend_extract_lexemes(ctx, &node_data, input_array, tokens, node_types);
            if (err)
                throw futhark::Error(ctx);
        });
        input_array.clear();
        tokens.clear();

        auto resolution = futhark::UniqueArray<int32_t, 1>(ctx);
        p.measure("resolve vars", [&]{
            bool valid;
            int err = futhark_entry_frontend_resolve_vars(ctx, &valid, &resolution, node_types, parents, prev_siblings, node_data);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::INVALID_VARIABLE);
        });

        p.measure("resolve fns", [&]{
            auto old_resolution = std::move(resolution);
            bool valid;
            int err = futhark_entry_frontend_resolve_fns(ctx, &valid, &resolution, node_types, old_resolution, node_data);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::DUPLICATE_FN_OR_INVALID_CALL);
        });

        p.measure("resolve args", [&]{
            auto old_resolution = std::move(resolution);
            bool valid;
            int err = futhark_entry_frontend_resolve_args(ctx, &valid, &resolution, node_types, parents, prev_siblings, old_resolution);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::INVALID_ARG_COUNT);
        });

        auto data_types = futhark::UniqueArray<uint8_t, 1>(ctx);
        p.measure("resolve dtypes", [&]{
            bool valid;
            int err = futhark_entry_frontend_resolve_data_types(ctx, &valid, &data_types, node_types, parents, prev_siblings, resolution.get());
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::TYPE_ERROR);
        });

        p.measure("check return dtypes", [&]{
            bool valid;
            int err = futhark_entry_frontend_check_return_types(ctx, &valid, node_types, parents, data_types);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::INVALID_RETURN);
        });

        p.measure("check convergence", [&]{
            bool valid;
            int err = futhark_entry_frontend_check_convergence(ctx, &valid, node_types, parents, prev_siblings, data_types);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Error::MISSING_RETURN);
        });

        auto ast = DeviceAst(ctx);
        p.measure("build ast", [&]{
            // Other arrays are destructed at the end of the function.
            int err = futhark_entry_frontend_build_ast(
                ctx,
                &ast.node_types,
                &ast.parents,
                &ast.node_data,
                &ast.data_types,
                &ast.node_depths,
                &ast.child_indexes,
                &ast.fn_tab,
                node_types,
                parents,
                node_data,
                data_types,
                prev_siblings,
                resolution
            );
            if (err)
                throw futhark::Error(ctx);
        });

        p.end("sema");
        p.end("compile");

        return ast;
    }
}
