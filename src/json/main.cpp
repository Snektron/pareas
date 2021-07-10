#include "json_futhark_generated.h"
#include "json_grammar.hpp"

#include "pareas/json/futhark_interop.hpp"
#include "pareas/profiler/profiler.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>

#include <iostream>
#include <fstream>
#include <charconv>
#include <cstdlib>

// This file is mostly just copied from src/compiler/main.cpp

struct Options {
    const char* input_path;
    bool help;
    bool futhark_verbose;
    bool futhark_debug;

    // Options available for the multicore backend
    int threads;

    // Options abailable for the OpenCL and CUDA backends
    const char* device_name;
    bool futhark_profile;
};

void print_usage(char* progname) {
    fmt::print(
        "Usage: {} [options...] <input path>\n"
        "Available options:\n"
        "-h --help                   Show this message and exit.\n"
        "--futhark-verbose           Enable Futhark logging.\n"
        "--futhark-debug             Enable Futhark debug logging.\n"
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
        "--futhark-profile           Enable Futhark profiling and print report at exit.\n"
    #endif
        "\n"
        "When <input path> is '-', standard input is used\n",
        progname
    );
}

bool parse_options(Options* opts, int argc, char* argv[]) {
    *opts = {
        .input_path = nullptr,
        .help = false,
        .futhark_verbose = false,
        .futhark_debug = false,
        .threads = 0,
        .device_name = nullptr,
        .futhark_profile = false,
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

        if (arg == "-h" || arg == "--help") {
            opts->help = true;
        } else if (arg == "-v" || arg == "--verbose") {
            opts->futhark_verbose = true;
        } else if (arg == "--futhark-debug") {
            opts->futhark_debug = true;
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

template <typename T>
struct Free {
    void operator()(T* ptr) const {
        free(static_cast<void*>(ptr));
    }
};

template <typename T>
using MallocPtr = std::unique_ptr<T, Free<T>>;

futhark::UniqueLexTable upload_lex_table(futhark::Context& ctx) {
    auto initial_state = futhark::UniqueArray<uint16_t, 1>(
        ctx,
        reinterpret_cast<const json::LexTable::State*>(json::lex_table.initial_states),
        json::LexTable::NUM_INITIAL_STATES
    );

    auto merge_table = futhark::UniqueArray<uint16_t, 2>(
        ctx,
        reinterpret_cast<const json::LexTable::State*>(json::lex_table.merge_table),
        json::lex_table.n,
        json::lex_table.n
    );

    auto final_state = futhark::UniqueArray<uint8_t, 1>(
        ctx,
        reinterpret_cast<const std::underlying_type_t<json::Token>*>(json::lex_table.final_states),
        json::lex_table.n
    );

    auto lex_table = futhark::UniqueLexTable(ctx);

    int err = futhark_entry_mk_lex_table(
        ctx.get(),
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
T upload_strtab(futhark::Context& ctx, const json::StrTab<U>& strtab, F upload_fn) {
    static_assert(sizeof(U) == sizeof(uint8_t));

    auto table = futhark::UniqueArray<uint8_t, 1>(
        ctx,
        reinterpret_cast<const uint8_t*>(strtab.table),
        strtab.n
    );

    auto offsets = futhark::UniqueArray<int32_t, 2>(ctx, strtab.offsets, json::NUM_TOKENS, json::NUM_TOKENS);
    auto lengths = futhark::UniqueArray<int32_t, 2>(ctx, strtab.lengths, json::NUM_TOKENS, json::NUM_TOKENS);

    auto tab = T(ctx);

    int err = upload_fn(ctx.get(), &tab, table.get(), offsets.get(), lengths.get());
    if (err != 0)
        throw futhark::Error(ctx);

    return tab;
}

int main(int argc, char* argv[]) {
    Options opts;
    if (!parse_options(&opts, argc, argv)) {
        fmt::print(std::cerr, "See '{} --help' for usage\n", argv[0]);
        return EXIT_FAILURE;
    } else if (opts.help) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    auto p = pareas::Profiler(9999);

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
    futhark_context_config_set_debugging(config.get(), opts.futhark_debug);

    #if defined(FUTHARK_BACKEND_multicore)
        futhark_context_config_set_num_threads(config.get(), opts.threads);
    #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
        if (opts.device_name) {
            futhark_context_config_set_device(config.get(), opts.device_name);
        }

        futhark_context_config_set_profiling(config.get(), opts.futhark_profile);
    #endif

    auto ctx = futhark::Context(futhark_context_new(config.get()));
    p.set_sync_callback([ctx = ctx.get()]{
        if (futhark_context_sync(ctx))
            throw futhark::Error(ctx);
    });
    p.end("context init");

    p.begin();
    auto lex_table = upload_lex_table(ctx);

    auto sct = upload_strtab<futhark::UniqueStackChangeTable>(
        ctx,
        json::stack_change_table,
        futhark_entry_mk_stack_change_table
    );

    auto pt = upload_strtab<futhark::UniqueParseTable>(
        ctx,
        json::parse_table,
        futhark_entry_mk_parse_table
    );

    auto arity_array = futhark::UniqueArray<int32_t, 1>(ctx, json::arities, json::NUM_PRODUCTIONS);
    p.end("table upload");

    p.begin();
    auto input_array = futhark::UniqueArray<uint8_t, 1>(ctx, reinterpret_cast<const uint8_t*>(input.data()), input.size());
    p.end("input upload");

    if (futhark_context_sync(ctx.get()) != 0)
        throw futhark::Error(ctx);

    p.begin();
    bool result;
    int err = futhark_entry_main(
        ctx.get(),
        &result,
        input_array.get(),
        lex_table.get(),
        sct.get(),
        pt.get(),
        arity_array.get()
    );
    p.end("parse");

    p.dump(std::cout);

    if (err != 0)
        throw futhark::Error(ctx);

    if (opts.futhark_profile) {
        auto report = MallocPtr<char>(futhark_context_report(ctx.get()));
        fmt::print("Profile report:\n{}", report);
    }

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
