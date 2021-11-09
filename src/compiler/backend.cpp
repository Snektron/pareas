#include "pareas/compiler/backend.hpp"
#include "pareas/compiler/futhark_interop.hpp"

namespace backend {
    DeviceModule compile(futhark_context* ctx, DeviceAst& ast, pareas::Profiler& p) {
        auto tree = futhark::UniqueTree(ctx);
        p.measure("translate ast", [&] {
            int err = futhark_entry_backend_convert_tree(
                ctx,
                &tree,
                ast.node_types,
                ast.parents,
                ast.node_data,
                ast.data_types,
                ast.node_depths,
                ast.child_indexes
            );
            if(err)
                throw futhark::Error(ctx);
        });

        // Stage 1, preprocessing
        p.measure("preprocessing", [&] {
            auto old_tree = std::move(tree);

            int err = futhark_entry_backend_preprocess(ctx, &tree, old_tree);
            if(err)
                throw futhark::Error(ctx);
        });

        // Stage 2, instruction count
        auto instr_counts = futhark::UniqueArray<uint32_t, 1>(ctx);
        auto functab = futhark::UniqueFuncInfoArray(ctx);
        p.measure("instruction count", [&] {
            auto sub_func_id = futhark::UniqueArray<uint32_t, 1>(ctx);
            auto sub_func_start = futhark::UniqueArray<uint32_t, 1>(ctx);
            auto sub_func_size = futhark::UniqueArray<uint32_t, 1>(ctx);

            int err = futhark_entry_backend_instr_count(
                ctx,
                &instr_counts,
                tree
            );
            if(err)
                throw futhark::Error(ctx);

            err = futhark_entry_backend_instr_count_make_function_table(
                ctx,
                &sub_func_id,
                &sub_func_start,
                &sub_func_size,
                tree,
                instr_counts
            );
            if(err)
                throw futhark::Error(ctx);

            err = futhark_entry_backend_compact_functab(
                ctx,
                &functab,
                sub_func_id,
                sub_func_start,
                sub_func_size
            );

            if(err)
                throw futhark::Error(ctx);
        });

        // Stage 3, instruction gen
        auto instr = futhark::UniqueInstrArray(ctx);
        p.measure("instruction gen", [&] {
            int err = futhark_entry_backend_instr_gen(
                ctx,
                &instr,
                tree,
                instr_counts,
                functab
            );
            if(err)
                throw futhark::Error(ctx);
        });

        //Print instr gen
        auto instr_gen_opcodes = futhark::UniqueArray<uint32_t, 1>(ctx);
        auto instr_gen_rd = futhark::UniqueArray<int64_t, 1>(ctx);
        auto instr_gen_rs1 = futhark::UniqueArray<int64_t, 1>(ctx);
        auto instr_gen_rs2 = futhark::UniqueArray<int64_t, 1>(ctx);
        auto instr_gen_jt = futhark::UniqueArray<uint32_t, 1>(ctx);
        int err = futhark_entry_backend_split_instr(
            ctx,
            &instr_gen_opcodes,
            &instr_gen_rd,
            &instr_gen_rs1,
            &instr_gen_rs2,
            &instr_gen_jt,
            instr
            );


        // Stage 4, optimizer
        auto optimize = futhark::UniqueArray<bool, 1>(ctx);
        p.measure("optimize", [&] {
            auto old_instr = std::move(instr);
            auto old_functab = std::move(functab);

            int err = futhark_entry_backend_optimize(
                ctx,
                &instr,
                &functab,
                &optimize,
                old_instr,
                old_functab
            );
            if(err)
                throw futhark::Error(ctx);
        });

        // Stage 5-6, regalloc + instr remove
        p.measure("regalloc/instr remove", [&] {
            auto old_instr = std::move(instr);
            auto old_functab = std::move(functab);

            int err = futhark_entry_backend_regalloc(
                ctx,
                &instr,
                &functab,
                old_instr,
                old_functab,
                ast.fn_tab,
                optimize
            );
            if(err)
                throw futhark::Error(ctx);
        });

        // Stage 7, jump fix
        auto mod = DeviceModule(ctx);
        p.measure("Jump Fix", [&] {
            auto old_instr = std::move(instr);

            int err = futhark_entry_backend_fix_jumps(
                ctx,
                &instr,
                &mod.func_id,
                &mod.func_start,
                &mod.func_size,
                old_instr,
                functab
            );
            if(err)
                throw futhark::Error(ctx);
        });

        // Stage 8, postprocess
        p.measure("postprocess", [&] {
            int err = futhark_entry_backend_postprocess(
                ctx,
                &mod.instructions,
                instr
            );
            if(err)
                throw futhark::Error(ctx);
        });

        return mod;
    }
}
