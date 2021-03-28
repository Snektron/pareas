#include "futhark_generated.h"
#include "pareas_grammar.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <string_view>
#include <memory>
#include <charconv>
#include <cstdlib>
#include <cstdint>
#include <cstring>

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
    fmt::print(
        "Usage: {} [options...] <input path>\n"
        "Available options:\n"
        "-o --output <output path>   Write the output to <output path>. (default: b.out)\n"
        "-h --help                   Show this message and exit.\n"
        "-v --verbose                Enable Futhark logging.\n"
        "-d --debug                  Enable Futhark debug logging.\n"
    #if defined(FUTHARK_BACKEND_multicore)
        "Available backend options:\n"
        "-t --threads <amount>       Set the maximum number of threads that may be used\n"
        "                            (default: amount of cores).\n"
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
        "output are used respectively.\n",
        progname
    );
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
                    fmt::print(std::cerr, "Error: Expected argument <amount> to option {}\n", arg);
                    return false;
                }

                threads_arg = argv[i];
                continue;
            }
        #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
            if (arg == "--device") {
                if (++i >= argc) {
                    fmt::print(std::cerr, "Error: Expected argument <name> to option {}\n", arg);
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
                fmt::print(std::cerr, "Error: Expected argument <output> to option {}\n", arg);
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

int main(int argc, const char* argv[]) {
    Options opts;
    if (!parse_options(&opts, argc, argv)) {
        fmt::print(std::cerr, "See '{} --help' for usage\n", argv[0]);
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

    auto context = UniqueCPtr<futhark_context, futhark_context_free>(futhark_context_new(config.get()));

    fmt::print("oef: {}\n", grammar::lex_table.initial_states[0]);

    // int64_t result;
    // int err = futhark_entry_main(context.get(), &result, 10);
    // if (!err)
    //     err = futhark_context_sync(context.get());

    // if (err) {
    //     auto err = MallocPtr<char>(futhark_context_get_error(context.get()));
    //     fmt::print(std::cerr, "Futhark error: {}", err ? err.get() : "(no diagnostic)");
    //     return EXIT_FAILURE;
    // }

    // fmt::print("Result: {}", result);

    if (opts.profile) {
        auto report = MallocPtr<char>(futhark_context_report(context.get()));
        fmt::print("Profile report:\n{}", report);
    }

    return EXIT_SUCCESS;
}
