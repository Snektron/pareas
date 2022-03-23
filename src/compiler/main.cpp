#include "futhark_generated.h"
#include "pareas_grammar.hpp"

#include "pareas/compiler/futhark_interop.hpp"
#include "pareas/compiler/ast.hpp"
#include "pareas/compiler/frontend.hpp"
#include "pareas/compiler/backend.hpp"
#include "pareas/profiler/profiler.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>

#include <iostream>
#include <fstream>
#include <string_view>
#include <memory>
#include <chrono>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>

struct Options {
    const char* input_path;
    const char* output_path;
    bool help;
    bool dump_dot;
    unsigned profile;
    bool check;
    bool verbose_tree;
    bool verbose_mod;
    bool futhark_verbose;
    bool futhark_debug;
    bool futhark_debug_extra;

    // Options available for the multicore backend
    int threads;

    // Options abailable for the OpenCL and CUDA backends
    const char* device_name;
    bool futhark_profile;
};

void print_usage(char* progname) {
    #if defined(FUTHARK_BACKEND_c)
        const char* backend = "c";
    #elif defined(FUTHARK_BACKEND_cuda)
        const char* backend = "CUDA";
    #elif defined(FUTHARK_BACKEND_opencl)
        const char* backend = "OpenCL";
    #elif defined(FUTHARK_BACKEND_multicore)
        const char* backend = "multicore";
    #endif

    fmt::print(
        "Usage: {} [options...] <input path>\n"
        "Available options:\n"
        "-o --output <output path>   Write the output to <output path>. (default: b.out)\n"
        "-h --help                   Show this message and exit.\n"
        "--dump-dot                  Dump tree as dot graph.\n"
        "-p --profile <level>        Record (non-futhark) profiling information.\n"
        "--check                     Only run check the program for validity; do not\n"
        "                            attempt to generate code.\n"
        "--verbose-tree              Dump some information about the tree to stderr.\n"
        "                            (default: 0, =disabled)\n"
        "--verbose-mod               Dump some information about the final module to\n"
        "                            stderr.\n"
        "--futhark-verbose           Enable Futhark logging.\n"
        "--futhark-debug             Enable Futhark debug logging.\n"
        "--futhark-debug-extra       Futhark debug logging with extra information.\n"
        "                            Not compatible with --futhark-debug.\n"
    #if defined(FUTHARK_BACKEND_multicore)
        "Available backend options:\n"
        "-t --threads <amount>       Set the maximum number of threads that may be used\n"
        "                            (default: amount of cores).\n"
    #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
        "Available backend options:\n"
        "-d --device <name>          Select the device that kernels are executed on. Any\n"
        "                            device which name contains <name> may be used. The\n"
        "                            special value #k may be used to select the k-th\n"
        "                            device reported by the platform.\n"
        "                            This value may also be set via the PAREAS_DEVICE\n"
        "                            environment variable.\n"
        "--futhark-profile           Enable Futhark profiling and print report at exit.\n"
    #endif
        "\n"
        "When <input path> and/or <output path> are '-', standard input and standard\n"
        "output are used respectively.\n"
        "Backend: {}\n",
        progname, backend
    );
}

bool parse_options(Options* opts, int argc, char* argv[]) {
    *opts = {
        .input_path = nullptr,
        .output_path = "b.out",
        .help = false,
        .dump_dot = false,
        .profile = 0,
        .verbose_tree = false,
        .verbose_mod = false,
        .futhark_verbose = false,
        .futhark_debug = false,
        .futhark_debug_extra = false,
        .threads = 0,
        .device_name = nullptr,
        .futhark_profile = false,
    };

    const char* threads_arg = nullptr;
    const char* profile_arg = nullptr;

    for (int i = 1; i < argc; ++i) {
        auto arg = std::string_view(argv[i]);

        #if defined(FUTHARK_BACKEND_multicore)
            if (arg == "-t" || arg == "--threads") {
                if (++i >= argc) {
                    fmt::print(std::cerr, "Error: Expected argument <amount> to option {}\n", arg);
                    return false;
                }

                threads_arg = argv[i];
                continue;
            }
        #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
            if (arg == "-d" || arg == "--device") {
                if (++i >= argc) {
                    fmt::print(std::cerr, "Error: Expected argument <name> to option {}\n", arg);
                    return false;
                }

                opts->device_name = argv[i];
                continue;
            } else if (arg == "--futhark-profile") {
                opts->futhark_profile = true;
                continue;
            }
        #endif

        if (arg == "-o" || arg == "--output") {
            if (++i >= argc) {
                fmt::print(std::cerr, "Error: Expected argument <output> to option {}\n", arg);
                return false;
            }
            opts->output_path = argv[i];
        } else if (arg == "-h" || arg == "--help") {
            opts->help = true;
        } else if (arg == "--dump-dot") {
            opts->dump_dot = true;
        } else if (arg == "-p" || arg == "--profile") {
            if (++i >= argc) {
                fmt::print(std::cerr, "Error: Expected argument <level> to option {}\n", arg);
                return false;
            }

            profile_arg = argv[i];
        } else if (arg == "--check") {
            opts->check = true;
        } else if (arg == "--verbose-tree") {
            opts->verbose_tree = true;
        } else if (arg == "--verbose-mod") {
            opts->verbose_mod = true;
        } else if (arg == "--futhark-verbose") {
            opts->futhark_verbose = true;
        } else if (arg == "--futhark-debug") {
            opts->futhark_debug = true;
        } else if (arg == "--futhark-debug-extra") {
            opts->futhark_debug_extra = true;
        } else if (!opts->input_path) {
            opts->input_path = argv[i];
        } else {
            fmt::print(std::cerr, "Error: Unknown option {}\n", arg);
            return false;
        }
    }

    if (opts->help)
        return true;

    if (!opts->input_path) {
        fmt::print(std::cerr, "Error: Missing required argument <input path>\n");
        return false;
    } else if (!opts->input_path[0]) {
        fmt::print(std::cerr, "Error: <input path> may not be empty\n");
        return false;
    } else if (opts->futhark_debug && opts->futhark_debug_extra) {
        fmt::print(std::cerr, "Error: --futhark-debug is incompatible with --futhark-debug-extra\n");
        return false;
    }

    if (!opts->output_path[0]) {
        fmt::print(std::cerr, "Error: <output path> may not be empty\n");
        return false;
    }

    if (threads_arg) {
        const auto* end = threads_arg + std::strlen(threads_arg);
        auto [p, ec] = std::from_chars(threads_arg, end, opts->threads);
        if (ec != std::errc() || p != end || opts->threads < 1) {
            fmt::print(std::cerr, "Error: Invalid value '{}' for option --threads\n", threads_arg);
            return false;
        }
    }

    if (profile_arg) {
        const auto* end = profile_arg + std::strlen(profile_arg);
        auto [p, ec] = std::from_chars(profile_arg, end, opts->profile);
        if (ec != std::errc() || p != end) {
            fmt::print(std::cerr, "Error: Invalid value '{}' for option --profile\n", profile_arg);
            return false;
        }
    }

    return true;
}

template <typename T>
struct Free {
    void operator()(T* ptr) const {
        free(static_cast<void*>(ptr));
    }
};

template <typename T>
using MallocPtr = std::unique_ptr<T, Free<T>>;

int main(int argc, char* argv[]) {
    Options opts;
    if (!parse_options(&opts, argc, argv)) {
        fmt::print(std::cerr, "See '{} --help' for usage\n", argv[0]);
        return EXIT_FAILURE;
    } else if (opts.help) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    auto p = pareas::Profiler(opts.profile);

    auto in = std::ifstream(opts.input_path, std::ios::binary);
    if (!in) {
        fmt::print(std::cerr, "Error: Failed to open input file '{}'\n", opts.input_path);
        return EXIT_FAILURE;
    }

    auto input = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    in.close();

    p.begin();
    auto config = futhark::ContextConfig(futhark_context_config_new());
    futhark_context_config_set_logging(config.get(), opts.futhark_verbose);
    futhark_context_config_set_debugging(config.get(), opts.futhark_debug || opts.futhark_debug_extra);

    #if defined(FUTHARK_BACKEND_multicore)
        futhark_context_config_set_num_threads(config.get(), opts.threads);
    #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
        const char* device_name = opts.device_name ? opts.device_name : std::getenv("PAREAS_DEVICE");
        if (opts.device_name) {
            futhark_context_config_set_device(config.get(), device_name);
        }

        futhark_context_config_set_profiling(config.get(), opts.futhark_profile);
    #endif

    auto ctx = futhark::Context(futhark_context_new(config.get()));
    futhark_context_set_logging_file(ctx.get(), stderr);
    p.set_sync_callback([ctx = ctx.get()]{
        if (futhark_context_sync(ctx))
            throw futhark::Error(ctx);
    });
    p.end("context init");

    try {
        p.begin();
        auto ast = frontend::compile(ctx.get(), input, opts.verbose_tree, p, opts.futhark_debug_extra ? stderr : nullptr);
        p.end("frontend");

        if (opts.dump_dot) {
            p.begin();
            auto host_ast = ast.download();
            p.end("ast download");
            host_ast.dump_dot(std::cout);
        }

        if (!opts.check) {
            p.begin();
            auto module = backend::compile(ctx.get(), ast, p);
            p.end("backend");

            auto host_mod = module.download();

            if (opts.verbose_mod) {
                host_mod.dump(std::cerr);
            }

            auto out = std::ofstream(opts.output_path, std::ios::binary);
            if (!out) {
                fmt::print(std::cerr, "Failed to open output file '{}'\n", opts.output_path);
                return EXIT_FAILURE;
            }

            out.write(reinterpret_cast<const char*>(host_mod.instructions.get()), host_mod.num_instructions * sizeof(uint32_t));
        }

        if (opts.profile > 0)
            p.dump(std::cout);

        if (opts.futhark_profile) {
            auto report = MallocPtr<char>(futhark_context_report(ctx.get()));
            fmt::print(std::cerr, "Futhark profile report:\n{}", report);
        }
    } catch (const frontend::CompileError& err) {
        fmt::print(std::cerr, "Compile error: {}\n", err.what());
        return EXIT_FAILURE;
    } catch (const futhark::Error& err) {
        fmt::print(std::cerr, "Futhark error: {}\n", err.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
