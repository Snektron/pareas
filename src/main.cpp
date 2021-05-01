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
    bool profile;
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
        .profile = false,
    };

    const char* threads_arg = nullptr;

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
            } else if (arg == "-p" || arg == "--profile") {
                opts->profile = true;
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
            deleter(this->ctx, this->data);
        }

        T* get() {
            return this->data;
        }

        T** operator&() {
            return &this->data;
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

    try {
        std::ifstream input(opts.input_path);
        Lexer lexer(input);
        SymbolTable symtab;
        Parser parser(lexer, symtab);

        std::unique_ptr<ASTNode> node(parser.parse());
        std::cout << *node << std::endl;

        std::cout << symtab << std::endl;

        node->resolveType();
        std::cout << *node << std::endl;

        DepthTree depth_tree(node.get());
        std::cout << depth_tree << std::endl;

        auto context = UniqueCPtr<futhark_context, futhark_context_free>(futhark_context_new(config.get()));

        auto node_types = UniqueFPtr<futhark_u8_1d, futhark_free_u8_1d>(context.get(),
                            futhark_new_u8_1d(context.get(), depth_tree.getNodeTypes(), depth_tree.maxNodes()));
        auto resulting_types = UniqueFPtr<futhark_u8_1d, futhark_free_u8_1d>(context.get(),
                            futhark_new_u8_1d(context.get(), depth_tree.getResultingTypes(), depth_tree.maxNodes()));
        auto parents = UniqueFPtr<futhark_i32_1d, futhark_free_i32_1d>(context.get(),
                            futhark_new_i32_1d(context.get(), depth_tree.getParents(), depth_tree.maxNodes()));
        auto depth = UniqueFPtr<futhark_i32_1d, futhark_free_i32_1d>(context.get(),
                            futhark_new_i32_1d(context.get(), depth_tree.getDepth(), depth_tree.maxNodes()));
        auto child_idx = UniqueFPtr<futhark_i32_1d, futhark_free_i32_1d>(context.get(),
                            futhark_new_i32_1d(context.get(), depth_tree.getChildren(), depth_tree.maxNodes()));
        auto node_data = UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d>(context.get(),
                            futhark_new_u32_1d(context.get(), depth_tree.getNodeData(), depth_tree.maxNodes()));

        UniqueFPtr<futhark_opaque_Tree, futhark_free_opaque_Tree> gpu_tree(context.get());
        int err = futhark_entry_make_tree(context.get(), &gpu_tree, depth_tree.maxDepth(), node_types.get(), resulting_types.get(),
                                            parents.get(), depth.get(), child_idx.get(), node_data.get());

        auto symtab_types = UniqueFPtr<futhark_u8_1d, futhark_free_u8_1d>(context.get(),
                            futhark_new_u8_1d(context.get(), symtab.getDataTypes(), symtab.maxVars()));
        auto symtab_offsets = UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d>(context.get(),
                            futhark_new_u32_1d(context.get(), symtab.getOffsets(), symtab.maxVars()));

        UniqueFPtr<futhark_opaque_Symtab, futhark_free_opaque_Symtab> gpu_symtab(context.get());
        if(!err)
            err = futhark_entry_make_symtab(context.get(), &gpu_symtab, symtab_types.get(), symtab_offsets.get());

        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> instr_offsets(context.get());
        if(!err)
            err = futhark_entry_make_instr_counts(context.get(), &instr_offsets, gpu_tree.get());

        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> function_ids(context.get());
        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> function_offsets(context.get());
        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> function_sizes(context.get());
        if(!err)
            err = futhark_entry_make_function_table(context.get(), &function_ids, &function_offsets, &function_sizes, gpu_tree.get(), instr_offsets.get());

        UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d> instr_fut(context.get());
        UniqueFPtr<futhark_i64_1d, futhark_free_i64_1d> rd_fut(context.get());
        UniqueFPtr<futhark_i64_1d, futhark_free_i64_1d> rs1_fut(context.get());
        UniqueFPtr<futhark_i64_1d, futhark_free_i64_1d> rs2_fut(context.get());
        if(!err)
            err = futhark_entry_main(context.get(), &instr_fut, &rd_fut, &rs1_fut, &rs2_fut, gpu_tree.get(), gpu_symtab.get(),
                            instr_offsets.get());
        
        UniqueFPtr<futhark_i32_1d, futhark_free_i32_1d> register_instr_offsets(context.get());
        UniqueFPtr<futhark_u64_1d, futhark_free_u64_1d> lifetime_masks(context.get());
        if(!err)
            err = futhark_entry_do_register_alloc(context.get(), &register_instr_offsets, &lifetime_masks, instr_fut.get(), rd_fut.get(), rs1_fut.get(), rs2_fut.get(),
                            function_ids.get(), function_offsets.get(), function_sizes.get());
        
        if (!err)
            err = futhark_context_sync(context.get());

        if (err) {
            auto err = MallocPtr<char>(futhark_context_get_error(context.get()));
            std::cerr << "Futhark error: " << (err ? err.get() : "(no diagnostic)") << std::endl;
            return EXIT_FAILURE;
        }

        size_t num_instr_counts = *futhark_shape_u32_1d(context.get(), instr_offsets.get());

        std::unique_ptr<uint32_t[]> instr_offset_buffer(new uint32_t[num_instr_counts]);
        if(!err)
            err = futhark_values_u32_1d(context.get(), instr_offsets.get(), instr_offset_buffer.get());

        size_t num_functab_entries = *futhark_shape_u32_1d(context.get(), function_ids.get());
        std::unique_ptr<uint32_t[]> functab_keys(new uint32_t[num_functab_entries]);
        std::unique_ptr<uint32_t[]> functab_values(new uint32_t[num_functab_entries]);
        std::unique_ptr<uint32_t[]> functab_sizes(new uint32_t[num_functab_entries]);

        if(!err)
            err = futhark_values_u32_1d(context.get(), function_ids.get(), functab_keys.get());
        if(!err)
            err = futhark_values_u32_1d(context.get(), function_offsets.get(), functab_values.get());
        if(!err)
            err = futhark_values_u32_1d(context.get(), function_sizes.get(), functab_sizes.get());

        size_t num_values = *futhark_shape_u32_1d(context.get(), instr_fut.get());

        std::unique_ptr<uint32_t[]> instr(new uint32_t[num_values]);
        std::unique_ptr<int64_t[]> rd(new int64_t[num_values]);
        std::unique_ptr<int64_t[]> rs1(new int64_t[num_values]);
        std::unique_ptr<int64_t[]> rs2(new int64_t[num_values]);
        err = futhark_values_u32_1d(context.get(), instr_fut.get(), instr.get());
        if(!err)
            err = futhark_values_i64_1d(context.get(), rd_fut.get(), rd.get());
        if(!err)
            err = futhark_values_i64_1d(context.get(), rs1_fut.get(), rs1.get());
        if(!err)
            err = futhark_values_i64_1d(context.get(), rs2_fut.get(), rs2.get());

        if(err) {
            auto err = MallocPtr<char>(futhark_context_get_error(context.get()));
            std::cerr << "Futhark error: " << (err ? err.get() : "(no diagnostic)") << std::endl;
            return EXIT_FAILURE;
        }

        std::unique_ptr<int32_t[]> reg_instr_offsets(new int32_t[num_values]);
        std::unique_ptr<uint64_t[]> reg_lifetime_masks(new uint64_t[num_functab_entries]);

        if(!err)
            futhark_values_i32_1d(context.get(), register_instr_offsets.get(), reg_instr_offsets.get());
        if(!err)
            futhark_values_u64_1d(context.get(), lifetime_masks.get(), reg_lifetime_masks.get());

        std::cout << std::endl << "Instruction offsets:" << std::endl;
        for(size_t i = 0; i < num_instr_counts; ++i) {
            std::cout << i << ": " << instr_offset_buffer[i] << std::endl;
        }

        std::cout << std::endl << "Function offsets: " << std::endl;
        for(size_t i = 0; i < num_functab_entries; ++i) {
            std::cout << functab_keys[i] << " -> " << functab_values[i] << ", " << functab_sizes[i] << std::endl;
        }

        std::cout << std::endl << "Register allocation data: " << std::endl;
        for(size_t i = 0; i < num_functab_entries; ++i) {
            std::cout << "Function " << functab_keys[i] << ": " << std::bitset<64>(reg_lifetime_masks[i]) << std::endl;
        }

        std::cout << std::endl << "Instructions:" << std::endl;        
        for(size_t i = 0; i < num_values; ++i) {
            std::cout << i
                << "+" << reg_instr_offsets[i]
                << "\t= " << std::bitset<32>(instr[i]) << " " << rd[i] << " " << rs1[i] << " " << rs2[i] << std::endl;
        }


        if (opts.profile) {
            auto report = MallocPtr<char>(futhark_context_report(context.get()));
            std::cout << "Profile report:\n" << report << std::endl;
        }
    }
    catch(const ParseException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}