#include <futhark-generated.h>
#include <iostream>
#include <string_view>
#include <memory>
#include <charconv>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <bitset>

#include "codegen/lexer.hpp"
#include "codegen/parser.hpp"
#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"
#include "codegen/depthtree.hpp"
#include "codegen/symtab.hpp"
#include "codegen/treeproperties.hpp"

#include "profiler/profiler.hpp"

//const size_t MAX_NODES = 32;
//const size_t MAX_VARS = 32;

struct Options {
    const char* input_path;
    const char* output_path;
    bool help;
    bool verbose;
    bool debug;

    // Options available for the multicore backend
    int threads;

    // Options abailable for the OpenCL and CUDA backends
    const char* device_name;
    unsigned profile;
};

inline std::string get_error_str(futhark_context* ctx) {
    auto err = futhark_context_get_error(ctx);
    if (err) {
        auto err_str = std::string(err);
        free(err); // leak if the string constructor throws, but whatever.
        return err_str;
    }

    return "(no diagnostic)";
}

class FutharkError : public std::runtime_error {
    public:
        FutharkError(futhark_context* ctx) : std::runtime_error(get_error_str(ctx)) {}
        virtual ~FutharkError() = default;
};

void print_usage(const char* progname) {
    std::cout <<
        "Usage: " << progname << " [options...] <input path>\n"
        "Available options:\n"
        "-o --output <output path>   Write the output to <output path>. (default: b.out)\n"
        "-h --help                   Show this message and exit.\n"
        "-v --verbose                Enable Futhark logging.\n"
        "-d --debug                  Enable Futhark debug logging.\n"
    #if defined(FUTHARK_BACKEND_multicore)
        "Available backend options:\n"
        "-t --threads <amount>      Set the maximum number of threads that may be used.\n"
    #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
        "Available backend options:\n"
        "--device <name>             Select the device that kernels are executed on. Any\n"
        "                            device which name contains <name> may be used. The\n"
        "                            special value #k may be used to select the k-th\n"
        "                            device reported by the platform.\n"
        "-p --profile                Enable Futhark profiling and print report at exit.\n"
    #endif
        "\n"
        "When <input path> and/or <output path> are '-', standard input and standard\n"
        "output are used respectively.\n";
}

bool parse_options(Options* opts, int argc, const char* argv[]) {
    *opts = {
        .input_path = nullptr,
        .output_path = "b.out",
        .help = false,
        .verbose = false,
        .debug = false,
        .threads = 0,
        .device_name = nullptr,
        .profile = 0,
    };

    const char* threads_arg = nullptr;
    const char* profile_arg = nullptr;

    for (int i = 1; i < argc; ++i) {
        auto arg = std::string_view(argv[i]);

        #if defined(FUTHARK_BACKEND_multicore)
            if (arg == "-t" || arg == "--threads") {
                if (++i >= argc) {
                    std::cerr << "Error: Expected argument <amount> to option " << arg << std::endl;
                    return false;
                }

                threads_arg = argv[i];
                continue;
            }
        #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
            if (arg == "--device") {
                if (++i >= argc) {
                    std::cerr << "Error: Expected argument <name> to option " << arg << std::endl;
                    return false;
                }

                opts->device_name = argv[i];
                continue;
            }
        #endif

        if (arg == "-o" || arg == "--output") {
            if (++i >= argc) {
                std::cerr << "Error: Expected argument <output path> to option " << arg << std::endl;
                return false;
            }
            opts->output_path = argv[i];
        } else if (arg == "-h" || arg == "--help") {
            opts->help = true;
        } else if (arg == "-v" || arg == "--verbose") {
            opts->verbose = true;
        } else if (arg == "-d" || arg == "--debug") {
            opts->debug = true;
        } else if (arg == "-p" || arg == "--profile") {
            if (++i >= argc) {
                std::cerr << "Error: Expected argument <level> to option " << arg << std::endl;;
                return false;
            }

            profile_arg = argv[i];
        } else if (!opts->input_path) {
            opts->input_path = argv[i];
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
            return false;
        }
    }

    if (opts->help)
        return true;

    if (!opts->input_path) {
        std::cerr << "Error: Missing required argument <input path>" << std::endl;
        return false;
    } else if (!opts->input_path[0]) {
        std::cerr << "Error: <input path> may not be empty" << std::endl;
        return false;
    }

    if (!opts->output_path[0]) {
        std::cerr << "Error: <output path> may not be empty" << std::endl;
        return false;
    }

    if (threads_arg) {
        const auto* end = threads_arg + std::strlen(threads_arg);
        auto [p, ec] = std::from_chars(threads_arg, end, opts->threads);
        if (ec != std::errc() || p != end || opts->threads < 1) {
            std::cerr << "Error: Invalid value '" << threads_arg << "' for option --threads" << std::endl;
            return false;
        }
    }

    if (profile_arg) {
        const auto* end = profile_arg + std::strlen(profile_arg);
        auto [p, ec] = std::from_chars(profile_arg, end, opts->profile);
        if (ec != std::errc() || p != end) {
            std::cerr << "Error: Invalid value " << profile_arg << " for option --profile" << std::endl;
            return false;
        }
    }

    return true;
}

template <typename T, void(*deleter)(T*)>
struct Deleter {
    void operator()(T* t) const {
        deleter(t);
    }
};

template <typename T, void(*deleter)(T*)>
using UniqueCPtr = std::unique_ptr<T, Deleter<T, deleter>>;

template <typename T>
struct Free {
    void operator()(T* ptr) const {
        free(static_cast<void*>(ptr));
    }
};

template <typename T>
using MallocPtr = std::unique_ptr<T, Free<T>>;

template <typename T, int(*deleter)(futhark_context*, T*)>
class UniqueFPtr {
    private:
        futhark_context* ctx;
        T* data;
    public:
        UniqueFPtr(futhark_context* ctx) : ctx(ctx), data(nullptr) {}
        UniqueFPtr(futhark_context* ctx, T* data) : ctx(ctx), data(data) {}
        UniqueFPtr(const UniqueFPtr&) = delete;
        UniqueFPtr(UniqueFPtr&& o) {
            std::swap(this->ctx, o.ctx);
            std::swap(this->data, o.data);
        }

        UniqueFPtr& operator=(const UniqueFPtr&) = delete;
        UniqueFPtr& operator=(UniqueFPtr&& o) {
            std::swap(this->ctx, o.ctx);
            std::swap(this->data, o.data);
        }

        ~UniqueFPtr() {
            if(this->data != nullptr)
                deleter(this->ctx, this->data);
        }

        T* get() {
            return this->data;
        }

        T** operator&() {
            return &this->data;
        }

        void release() {
            deleter(this->ctx, this->data);
            this->data = nullptr;
        }
};

int main(int argc, const char* argv[]) {
    Options opts;
    if (!parse_options(&opts, argc, argv)) {
        std::cerr << "See '" << argv[0] << " --help' for usage" << std::endl;
        return EXIT_FAILURE;
    } else if (opts.help) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    auto config = UniqueCPtr<futhark_context_config, futhark_context_config_free>(futhark_context_config_new());
    futhark_context_config_set_logging(config.get(), opts.verbose);
    futhark_context_config_set_debugging(config.get(), opts.debug);

    #if defined(FUTHARK_BACKEND_multicore)
        futhark_context_config_set_num_threads(config.get(), opts.threads);
    #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
        if (opts.device_name) {
            futhark_context_config_set_device(config.get(), opts.device_name);
        }

        futhark_context_config_set_profiling(config.get(), opts.profile);
    #endif

    auto p = pareas::Profiler(opts.profile);

    try {
        std::ifstream input(opts.input_path);
        if(!input) {
            std::cerr << "Failed to open file " << opts.input_path << std::endl;
            return EXIT_FAILURE;
        }
        p.begin();

        //Stage 0, CPU setup
        p.begin();
        Lexer lexer(input);
        SymbolTable symtab;
        Parser parser(lexer, symtab);

        std::unique_ptr<ASTNode> node(parser.parse());
        node->resolveType();

        DepthTree depth_tree(node.get());
        p.end("Setup");

        TreeProperties props(node.get());
        std::cout << "Number of nodes: " << props.getNodeCount() << std::endl;
        std::cout << "Tree width: " << props.getWidth() << std::endl;
        std::cout << "Tree height: " << props.getDepth() << std::endl;
        std::cout << "Num functions: " << props.getFunctions() << std::endl;
        std::cout << "Max function length: " << props.getMaxFuncLen() << std::endl;

        return 0;

        auto context = UniqueCPtr<futhark_context, futhark_context_free>(futhark_context_new(config.get()));

        p.set_sync_callback([ctx = context.get()] {
            if(futhark_context_sync(ctx))
                throw FutharkError(ctx);
        });

        //Start of GPU
        p.begin();

        //Stage 0, uploading data
        p.begin();
        auto stage0_node_types = UniqueFPtr<futhark_u8_1d, futhark_free_u8_1d>(context.get(),
                            futhark_new_u8_1d(context.get(), depth_tree.getNodeTypes(), depth_tree.maxNodes()));
        auto stage0_data_types = UniqueFPtr<futhark_u8_1d, futhark_free_u8_1d>(context.get(),
                            futhark_new_u8_1d(context.get(), depth_tree.getResultingTypes(), depth_tree.maxNodes()));
        auto stage0_parents = UniqueFPtr<futhark_i32_1d, futhark_free_i32_1d>(context.get(),
                            futhark_new_i32_1d(context.get(), depth_tree.getParents(), depth_tree.maxNodes()));
        auto stage0_depth = UniqueFPtr<futhark_i32_1d, futhark_free_i32_1d>(context.get(),
                            futhark_new_i32_1d(context.get(), depth_tree.getDepth(), depth_tree.maxNodes()));
        auto stage0_child_idx = UniqueFPtr<futhark_i32_1d, futhark_free_i32_1d>(context.get(),
                            futhark_new_i32_1d(context.get(), depth_tree.getChildren(), depth_tree.maxNodes()));
        auto stage0_node_data = UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d>(context.get(),
                            futhark_new_u32_1d(context.get(), depth_tree.getNodeData(), depth_tree.maxNodes()));
        auto stage0_symb_data_types = UniqueFPtr<futhark_u8_1d, futhark_free_u8_1d>(context.get(),
                            futhark_new_u8_1d(context.get(), symtab.getDataTypes(), symtab.maxVars()));
        auto stage0_symb_offsets = UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d>(context.get(),
                            futhark_new_u32_1d(context.get(), symtab.getOffsets(), symtab.maxVars()));
        auto stage0_function_symbols = UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d>(context.get(),
                            futhark_new_u32_1d(context.get(), symtab.getFuncVarCount(), symtab.numFuncs()));


        UniqueFPtr<futhark_opaque_Tree, futhark_free_opaque_Tree> stage0_tree(context.get());
        UniqueFPtr<futhark_opaque_Symtab, futhark_free_opaque_Symtab> stage0_symtab(context.get());
        p.measure("Create", [&] {
            int err = futhark_entry_make_tree(context.get(),
                                                        &stage0_tree,
                                                        depth_tree.maxDepth(),
                                                        stage0_node_types.get(),
                                                        stage0_data_types.get(),
                                                        stage0_parents.get(),
                                                        stage0_depth.get(),
                                                        stage0_child_idx.get(),
                                                        stage0_node_data.get());
            if(err)
                throw FutharkError(context.get());

            err = futhark_entry_make_symtab(context.get(),
                                                        &stage0_symtab,
                                                        stage0_symb_data_types.get(),
                                                        stage0_symb_offsets.get());
            if(err)
                throw FutharkError(context.get());
        });

        p.end("Upload");

        //Stage 1, preprocessing
        UniqueFPtr<futhark_opaque_Tree, futhark_free_opaque_Tree> stage1_tree(context.get());

        p.measure("Preprocessing", [&] {
            int err = futhark_entry_stage_preprocess(context.get(),
                                                        &stage1_tree,
                                                        stage0_tree.get());
            if(err)
                throw FutharkError(context.get());
        });

        //Stage 2, instruction count
        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> stage2_instr_counts(context.get());
        UniqueFPtr<futhark_opaque_arr_FuncInfo_1d, futhark_free_opaque_arr_FuncInfo_1d> stage2_functab(context.get());

        p.measure("Instruction count", [&] {
            UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> stage2_sub_func_id(context.get());
            UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> stage2_sub_func_start(context.get());
            UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> stage2_sub_func_size(context.get());

            int err = futhark_entry_stage_instr_count(context.get(),
                                                        &stage2_instr_counts,
                                                        stage1_tree.get());
            if(err)
                throw FutharkError(context.get());

            err = futhark_entry_stage_instr_count_make_function_table(context.get(),
                                                        &stage2_sub_func_id,
                                                        &stage2_sub_func_start,
                                                        &stage2_sub_func_size,
                                                        stage1_tree.get(),
                                                        stage2_instr_counts.get());
            if(err)
                throw FutharkError(context.get());

            err = futhark_entry_stage_compact_functab(context.get(),
                                                        &stage2_functab,
                                                        stage2_sub_func_id.get(),
                                                        stage2_sub_func_start.get(),
                                                        stage2_sub_func_size.get());
            if(err)
                throw FutharkError(context.get());
        });

        //Stage 3, instruction gen
        UniqueFPtr<futhark_opaque_arr_Instr_1d, futhark_free_opaque_arr_Instr_1d> stage3_instr(context.get());

        p.measure("Instruction gen", [&] {
            int err = futhark_entry_stage_instr_gen(context.get(),
                                                        &stage3_instr,
                                                        stage1_tree.get(),
                                                        stage0_symtab.get(),
                                                        stage2_instr_counts.get(),
                                                        stage2_functab.get());
            if(err)
                throw FutharkError(context.get());
        });

        //Stage 4, optimizer
        UniqueFPtr<futhark_opaque_arr_Instr_1d, futhark_free_opaque_arr_Instr_1d> stage4_instr(context.get());
        UniqueFPtr<futhark_opaque_arr_FuncInfo_1d, futhark_free_opaque_arr_FuncInfo_1d> stage4_functab(context.get());
        UniqueFPtr<futhark_bool_1d, futhark_free_bool_1d> stage4_optimize(context.get());
        p.measure("Optimize", [&] {
            int err = futhark_entry_stage_optimize(context.get(),
                                                        &stage4_instr,
                                                        &stage4_functab,
                                                        &stage4_optimize,
                                                        stage3_instr.get(),
                                                        stage2_functab.get());
            if(err)
                throw FutharkError(context.get());
        });

        //Stage 5-6, regalloc + instr remove
        UniqueFPtr<futhark_opaque_arr_Instr_1d, futhark_free_opaque_arr_Instr_1d> stage5_instr(context.get());
        UniqueFPtr<futhark_opaque_arr_FuncInfo_1d, futhark_free_opaque_arr_FuncInfo_1d> stage5_functab(context.get());
        p.measure("Regalloc,Instr remove", [&] {
            int err = futhark_entry_stage_regalloc(context.get(),
                                                        &stage5_instr,
                                                        &stage5_functab,
                                                        stage4_instr.get(),
                                                        stage4_functab.get(),
                                                        stage0_function_symbols.get(),
                                                        stage4_optimize.get());
            if(err)
                throw FutharkError(context.get());
        });

        //Stage 7, jump fix
        UniqueFPtr<futhark_opaque_arr_Instr_1d, futhark_free_opaque_arr_Instr_1d> stage7_instr(context.get());
        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> result_func_id(context.get());
        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> result_func_start(context.get());
        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> result_func_size(context.get());
        p.measure("Jump Fix", [&] {
            int err = futhark_entry_stage_fix_jumps(context.get(),
                                                        &stage7_instr,
                                                        &result_func_id,
                                                        &result_func_start,
                                                        &result_func_size,
                                                        stage5_instr.get(),
                                                        stage5_functab.get());
            if(err)
                throw FutharkError(context.get());
        });

        //Stage 8, postprocess
        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> result_instr(context.get());
        p.measure("Postprocess", [&] {
            int err = futhark_entry_stage_postprocess(context.get(),
                                                    &result_instr,
                                                    stage7_instr.get());
            if(err)
                throw FutharkError(context.get());
        });

        //End of GPU
        p.end("GPU");

        //End of everything
        p.end("Global");

        if (opts.profile) {
            p.dump(std::cout);
            // auto report = MallocPtr<char>(futhark_context_report(context.get()));
            // std::cout << "Profile report:\n" << report << std::endl;
        }
    }
    catch(const ParseException& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(const FutharkError& e) {
        std::cerr << "Futhark error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}