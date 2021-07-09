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

    struct Timer {
        using Clock = std::chrono::high_resolution_clock;

        futhark_context* ctx;
        Clock::time_point start;

        Timer(futhark_context* ctx):
            ctx(ctx) {}

        void reset() {
            if (futhark_context_sync(ctx))
                throw futhark::Error(ctx);

            this->start = Clock::now();
        }

        std::chrono::microseconds lap() {
            if (futhark_context_sync(ctx))
                throw futhark::Error(ctx);

            auto end = Clock::now();
            auto dif = end - this->start;
            this->start = end;
            return std::chrono::duration_cast<std::chrono::microseconds>(dif);
        }
    };
}

namespace frontend {
    const char* status_name(Status s) {
        switch (s) {
            case Status::OK: return "ok";
            case Status::PARSE_ERROR: return "Parse error";
            case Status::STRAY_ELSE_ERROR: return "Stray else/elif";
            case Status::INVALID_DECL: return "Declaration cannot be both function and variable";
            case Status::INVALID_PARAMS: return "Invalid function parameter list";
            case Status::INVALID_ASSIGN: return "Invalid assignment lvalue";
            case Status::INVALID_FN_PROTO: return "Invalid function prototype";
            case Status::DUPLICATE_FN_OR_INVALID_CALL: return "Duplicate function declaration or call to undefined function";
            case Status::INVALID_VARIABLE: return "Undeclared variable";
            case Status::INVALID_ARG_COUNT: return "Invalid amount of arguments for function call";
            case Status::TYPE_ERROR: return "Type error";
            case Status::INVALID_RETURN: return "Return expression has invalid type";
            case Status::MISSING_RETURN: return "Not all code paths in non-void function return a value";
        }
    }

    void CombinedStatistics::dump(std::ostream& os) const {
        fmt::print(os, "table upload time: {}\n", this->table_upload);
        fmt::print(os, "input upload time: {}\n", this->input_upload);
        fmt::print(os, "compile time: {}\n", this->compile);
        fmt::print(os, "total time: {}\n", this->total);
    }

    DeviceAst compile_combined(futhark_context* ctx, const std::string& input, CombinedStatistics& stats) {
        auto timer = Timer(ctx);
        auto start = Timer::Clock::now();

        timer.reset();
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
        stats.table_upload = timer.lap();

        auto input_array = futhark::UniqueArray<uint8_t, 1>(ctx, reinterpret_cast<const uint8_t*>(input.data()), input.size());
        stats.input_upload = timer.lap();

        auto ast = DeviceAst(ctx);
        Status status;

        int err = futhark_entry_main(
            ctx,
            reinterpret_cast<std::underlying_type_t<Status>*>(&status),
            &ast.node_types,
            &ast.parents,
            &ast.node_data,
            &ast.data_types,
            &ast.node_depths,
            &ast.child_indexes,
            &ast.fn_tab,
            input_array.get(),
            lex_table.get(),
            sct.get(),
            pt.get(),
            arity_array.get()
        );

        if (err)
            throw futhark::Error(ctx);

        if (status != Status::OK)
            throw CompileError(status);

        stats.compile = timer.lap();

        auto stop = Timer::Clock::now();
        stats.total = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        return ast;
    }

    void SeparateStatistics::dump(std::ostream& os) const {
        fmt::print(os, "table upload time: {}\n", this->table_upload);
        fmt::print(os, "input upload time: {}\n", this->input_upload);
        fmt::print(os, "tokenize time: {}\n", this->tokenize);
        fmt::print(os, "parse time: {}\n", this->parse);
        fmt::print(os, "build parse tree time: {}\n", this->build_parse_tree);
        fmt::print(os, "fix bin ops time: {}\n", this->fix_bin_ops);
        fmt::print(os, "fix conditionals time: {}\n", this->fix_if_else);
        fmt::print(os, "flatten lists time: {}\n", this->flatten_lists);
        fmt::print(os, "fix names time: {}\n", this->fix_names);
        fmt::print(os, "fix ascriptions time: {}\n", this->fix_ascriptions);
        fmt::print(os, "fix fn decls time: {}\n", this->fix_fn_decls);
        fmt::print(os, "fix args and params time: {}\n", this->fix_args_and_params);
        fmt::print(os, "fix decls time: {}\n", this->fix_decls);
        fmt::print(os, "remove marker nodes time: {}\n", this->remove_marker_nodes);
        fmt::print(os, "compute prev siblings time: {}\n", this->compute_prev_siblings);
        fmt::print(os, "check assignments time: {}\n", this->check_assignments);
        fmt::print(os, "insert derefs time: {}\n", this->insert_derefs);
        fmt::print(os, "extract lexemestime: {}\n", this->extract_lexemes);
        fmt::print(os, "resolve vars time: {}\n", this->resolve_vars);
        fmt::print(os, "resolve fns time: {}\n", this->resolve_fns);
        fmt::print(os, "resolve args time: {}\n", this->resolve_args);
        fmt::print(os, "resolve data types time: {}\n", this->resolve_data_types);
        fmt::print(os, "check return types time: {}\n", this->check_return_types);
        fmt::print(os, "check convergence time: {}\n", this->check_convergence);
        fmt::print(os, "build ast time: {}\n", this->build_ast);
        fmt::print(os, "total time: {}\n", this->total);
    }

    DeviceAst compile_separate(futhark_context* ctx, const std::string& input, SeparateStatistics& stats) {
        auto timer = Timer(ctx);
        auto start = Timer::Clock::now();

        timer.reset();
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
        stats.table_upload = timer.lap();

        auto input_array = futhark::UniqueArray<uint8_t, 1>(ctx, reinterpret_cast<const uint8_t*>(input.data()), input.size());
        stats.input_upload = timer.lap();


        auto tokens = futhark::UniqueTokenArray(ctx);
        {
            int err = futhark_entry_frontend_tokenize(ctx, &tokens, input_array, lex_table);
            if (err)
                throw futhark::Error(ctx);
            stats.tokenize = timer.lap();
        }

        auto node_types = futhark::UniqueArray<uint8_t, 1>(ctx);
        {
            bool valid = false;
            int err = futhark_entry_frontend_parse(ctx, &valid, &node_types, tokens, sct, pt);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::PARSE_ERROR);
            stats.parse = timer.lap();
        }

        auto parents = futhark::UniqueArray<int32_t, 1>(ctx);
        {
            int err = futhark_entry_frontend_build_parse_tree(ctx, &parents, node_types, arity_array);
            if (err)
                throw futhark::Error(ctx);
            stats.build_parse_tree = timer.lap();
        }

        {
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_fix_bin_ops(ctx, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            stats.fix_bin_ops = timer.lap();
        }

        {
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            bool valid;
            int err = futhark_entry_frontend_fix_if_else(ctx, &valid, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::STRAY_ELSE_ERROR);
            stats.fix_if_else = timer.lap();
        }

        {
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_flatten_lists(ctx, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            stats.flatten_lists = timer.lap();
        }

        {
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            bool valid;
            int err = futhark_entry_frontend_fix_names(ctx, &valid, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::INVALID_DECL);
            stats.fix_names = timer.lap();
        }

        {
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_fix_ascriptions(ctx, &parents, node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            stats.fix_ascriptions = timer.lap();
        }

        {
            auto old_parents = std::move(parents);
            bool valid;
            int err = futhark_entry_frontend_fix_fn_decls(ctx, &valid, &parents, node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::INVALID_FN_PROTO);
            stats.fix_fn_decls = timer.lap();
        }

        {
            auto old_node_types = std::move(node_types);
            int err = futhark_entry_frontend_fix_args_and_params(ctx, &node_types, old_node_types, parents);
            if (err)
                throw futhark::Error(ctx);
            stats.fix_args_and_params = timer.lap();
        }

        {
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            bool valid;
            int err = futhark_entry_frontend_fix_decls(ctx, &valid, &node_types, &parents, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::INVALID_DECL);
            stats.fix_decls = timer.lap();
        }

        {
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_remove_marker_nodes(ctx, &parents, node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            stats.remove_marker_nodes = timer.lap();
        }

        auto prev_siblings = futhark::UniqueArray<int32_t, 1>(ctx);
        {
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            int err = futhark_entry_frontend_compute_prev_sibling(ctx, &node_types, &parents, &prev_siblings, old_node_types, old_parents);
            if (err)
                throw futhark::Error(ctx);
            stats.compute_prev_siblings = timer.lap();
        }

        {
            bool valid;
            int err = futhark_entry_frontend_check_assignments(ctx, &valid, node_types, parents, prev_siblings);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::INVALID_ASSIGN);
            stats.check_assignments = timer.lap();
        }

        {
            auto old_node_types = std::move(node_types);
            auto old_parents = std::move(parents);
            auto old_prev_siblings = std::move(prev_siblings);
            int err = futhark_entry_frontend_insert_derefs(ctx, &node_types, &parents, &prev_siblings, old_node_types, old_parents, old_prev_siblings);
            if (err)
                throw futhark::Error(ctx);
            stats.insert_derefs = timer.lap();
        }

        auto node_data = futhark::UniqueArray<uint32_t, 1>(ctx);
        {
            int err = futhark_entry_frontend_extract_lexemes(ctx, &node_data, input_array, tokens, node_types);
            if (err)
                throw futhark::Error(ctx);
            stats.extract_lexemes = timer.lap();
        }

        auto resolution = futhark::UniqueArray<int32_t, 1>(ctx);
        {
            bool valid;
            int err = futhark_entry_frontend_resolve_vars(ctx, &valid, &resolution, node_types, parents, prev_siblings, node_data);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::INVALID_VARIABLE);
            stats.resolve_vars = timer.lap();
        }

        {
            auto old_resolution = std::move(resolution);
            bool valid;
            int err = futhark_entry_frontend_resolve_fns(ctx, &valid, &resolution, node_types, old_resolution, node_data);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::DUPLICATE_FN_OR_INVALID_CALL);
            stats.resolve_fns = timer.lap();
        }

        {
            auto old_resolution = std::move(resolution);
            bool valid;
            int err = futhark_entry_frontend_resolve_args(ctx, &valid, &resolution, node_types, parents, prev_siblings, old_resolution);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::INVALID_ARG_COUNT);
            stats.resolve_args = timer.lap();
        }

        auto data_types = futhark::UniqueArray<uint8_t, 1>(ctx);
        {
            bool valid;
            int err = futhark_entry_frontend_resolve_data_types(ctx, &valid, &data_types, node_types, parents, prev_siblings, resolution.get());
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::TYPE_ERROR);
            stats.resolve_data_types = timer.lap();
        }

        {
            bool valid;
            int err = futhark_entry_frontend_check_return_types(ctx, &valid, node_types, parents, data_types);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::INVALID_RETURN);
            stats.check_return_types = timer.lap();
        }

        {
            bool valid;
            int err = futhark_entry_frontend_check_convergence(ctx, &valid, node_types, parents, prev_siblings, data_types);
            if (err)
                throw futhark::Error(ctx);
            if (!valid)
                throw CompileError(Status::INVALID_RETURN);
            stats.check_convergence = timer.lap();
        }

        auto ast = DeviceAst(ctx);
        {
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
            stats.build_ast = timer.lap();
        }

        auto stop = Timer::Clock::now();
        stats.total = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        return ast;
    }
}
