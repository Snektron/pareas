#include <futhark-generated.h>
#include <iostream>
#include <string_view>
#include <memory>
#include <charconv>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fstream>

#include "codegen/lexer.hpp"
#include "codegen/parser.hpp"
#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"
#include "codegen/depthtree.hpp"

const size_t MAX_NODES = 1024;

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
        std::ifstream input(argv[1]);
        Lexer lexer(input);
        Parser parser(lexer);

        std::unique_ptr<ASTNode> node(parser.parse());
        std::cout << *node << std::endl;

        node->resolveType();
        std::cout << *node << std::endl;

        DepthTree depth_tree(MAX_NODES, node.get());
        std::cout << depth_tree << std::endl;

        auto context = UniqueCPtr<futhark_context, futhark_context_free>(futhark_context_new(config.get()));

        auto node_types = UniqueFPtr<futhark_u8_1d, futhark_free_u8_1d>(context.get(),
                            futhark_new_u8_1d(context.get(), depth_tree.getNodeTypes(), depth_tree.maxNodes()));
        auto resulting_types = UniqueFPtr<futhark_u8_1d, futhark_free_u8_1d>(context.get(),
                            futhark_new_u8_1d(context.get(), depth_tree.getResultingTypes(), depth_tree.maxNodes()));
        auto parents = UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d>(context.get(),
                            futhark_new_u32_1d(context.get(), depth_tree.getParents(), depth_tree.maxNodes()));
        auto depth = UniqueFPtr<futhark_u32_1d, futhark_free_u32_1d>(context.get(),
                            futhark_new_u32_1d(context.get(), depth_tree.getDepth(), depth_tree.maxNodes()));

        UniqueFPtr<futhark_opaque_Tree, futhark_free_opaque_Tree> gpu_tree(context.get());
        int err = futhark_entry_make_tree(context.get(), &gpu_tree, depth_tree.maxDepth(), node_types.get(), resulting_types.get(), parents.get(), depth.get());

        int64_t result;
        if(!err)
            err = futhark_entry_main(context.get(), &result, gpu_tree.get());
        if (!err)
            err = futhark_context_sync(context.get());

        if (err) {
            auto err = MallocPtr<char>(futhark_context_get_error(context.get()));
            std::cerr << "Futhark error: " << (err ? err.get() : "(no diagnostic)") << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "Result: " << result << std::endl;

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