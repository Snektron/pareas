#include "futhark_generated.h"
#include "pareas_grammar.hpp"

#include "pareas/compiler/futhark_interop.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <fstream>
#include <string_view>
#include <memory>
#include <charconv>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>

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

template <typename T>
struct Free {
    void operator()(T* ptr) const {
        free(static_cast<void*>(ptr));
    }
};

template <typename T>
using MallocPtr = std::unique_ptr<T, Free<T>>;

void report_futhark_error(futhark::Context& ctx, std::string_view msg) {
    auto err = MallocPtr<char>(futhark_context_get_error(ctx.get()));
    fmt::print(std::cerr, "Error: {}:\n{}", msg, err ? err.get() : "(no diagnostic)");
}

futhark_opaque_lex_table* upload_lex_table(futhark::Context& ctx) {
    auto* initial_state = futhark_new_u16_1d(
        ctx.get(),
        reinterpret_cast<const grammar::LexTable::State*>(grammar::lex_table.initial_states),
        grammar::LexTable::NUM_INITIAL_STATES
    );

    auto* merge_table = futhark_new_u16_2d(
        ctx.get(),
        reinterpret_cast<const grammar::LexTable::State*>(grammar::lex_table.merge_table),
        grammar::lex_table.n,
        grammar::lex_table.n
    );

    auto* final_state = futhark_new_u8_1d(
        ctx.get(),
        reinterpret_cast<const std::underlying_type_t<grammar::Token>*>(grammar::lex_table.final_states),
        grammar::lex_table.n
    );

    futhark_opaque_lex_table* lex_table = nullptr;

    if (initial_state && merge_table && final_state) {
        int err = futhark_entry_mk_lex_table(
            ctx.get(),
            &lex_table,
            initial_state,
            merge_table,
            final_state
        );
        if (err)
            report_futhark_error(ctx, "Failed to upload lex table");
    }

    if (initial_state)
        futhark_free_u16_1d(ctx.get(), initial_state);

    if (merge_table)
        futhark_free_u16_2d(ctx.get(), merge_table);

    if (final_state)
        futhark_free_u8_1d(ctx.get(), final_state);

    return lex_table;
}

template <typename T, typename U, typename F>
T* upload_strtab(futhark::Context& ctx, const grammar::StrTab<U>& strtab, F upload_fn) {
    static_assert(sizeof(U) == sizeof(uint8_t));

    auto* table = futhark_new_u8_1d(
        ctx.get(),
        reinterpret_cast<const uint8_t*>(strtab.table),
        strtab.n
    );

    auto* offsets = futhark_new_i32_2d(ctx.get(), strtab.offsets, grammar::NUM_TOKENS, grammar::NUM_TOKENS);
    auto* lengths = futhark_new_i32_2d(ctx.get(), strtab.lengths, grammar::NUM_TOKENS, grammar::NUM_TOKENS);

    T* tab = nullptr;

    if (table && offsets && lengths) {
        int err = upload_fn(ctx.get(), &tab, table, offsets, lengths);
        if (err)
            report_futhark_error(ctx, "Failed to upload string table");
    }

    if (table)
        futhark_free_u8_1d(ctx.get(), table);

    if (offsets)
        futhark_free_i32_2d(ctx.get(), offsets);

    if (lengths)
        futhark_free_i32_2d(ctx.get(), lengths);

    return tab;
}

void dump_parse_tree(const std::vector<grammar::Production>& nodes, const std::vector<int32_t>& parents) {
    fmt::print("digraph prog {{\n");

    for (size_t i = 0; i < nodes.size(); ++i) {
        auto node = nodes[i];
        auto parent = parents[i];

        fmt::print("node{} [label=\"{}\"];\n", i, grammar::production_name(node));

        if (parent >= 0) {
            fmt::print("node{} -> node{};\n", parent, i);
        } else {
            fmt::print("start{0} [style=invis];\nstart{0} -> node{0};\n", i);
        }
    }

    fmt::print("}}\n");
}

void download_and_parse_tree(futhark::Context& ctx, futhark_u8_1d* nodes, futhark_i32_1d* parents) {
    int64_t n = futhark_shape_u8_1d(ctx.get(), nodes)[0];
    assert(n == futhark_shape_i32_1d(ctx.get(), parents)[0]);

    auto host_nodes = std::vector<grammar::Production>(n);
    auto host_parents = std::vector<int32_t>(n);

    int err = futhark_values_u8_1d(
        ctx.get(),
        nodes,
        reinterpret_cast<std::underlying_type_t<grammar::Production>*>(host_nodes.data())
    );
    if (err) {
        report_futhark_error(ctx, "Failed to download node data");
        return;
    }

    err = futhark_values_i32_1d(ctx.get(), parents, host_parents.data());
    if (err) {
        report_futhark_error(ctx, "Failed to download parent data");
        return;
    }

    dump_parse_tree(host_nodes, host_parents);
}

void download_and_dump_tokens(futhark::Context& ctx, futhark_u8_1d* tokens) {
    int64_t n = futhark_shape_u8_1d(ctx.get(), tokens)[0];

    auto host_tokens = std::vector<grammar::Token>(n);

    int err = futhark_values_u8_1d(
        ctx.get(),
        tokens,
        reinterpret_cast<std::underlying_type_t<grammar::Token>*>(host_tokens.data())
    );
    if (err) {
        report_futhark_error(ctx, "Failed to download tokens");
        return;
    }

    for (auto token : host_tokens) {
        fmt::print("{}\n", grammar::token_name(token));
    }
}

int main(int argc, const char* argv[]) {
    Options opts;
    if (!parse_options(&opts, argc, argv)) {
        fmt::print(std::cerr, "See '{} --help' for usage\n", argv[0]);
        return EXIT_FAILURE;
    } else if (opts.help) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    auto in = std::ifstream(opts.input_path, std::ios::binary);
    if (!in) {
        fmt::print(std::cerr, "Error: Failed to open input file '{}'\n", opts.input_path);
        return EXIT_FAILURE;
    }

    auto input = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    in.close();

    auto config = futhark::ContextConfig(futhark_context_config_new());

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

    auto ctx = futhark::Context(futhark_context_new(config.get()));

    auto* lex_table = upload_lex_table(ctx);
    auto* sct = upload_strtab<futhark_opaque_stack_change_table>(
        ctx,
        grammar::stack_change_table,
        futhark_entry_mk_stack_change_table
    );

    auto* pt = upload_strtab<futhark_opaque_parse_table>(
        ctx,
        grammar::parse_table,
        futhark_entry_mk_parse_table
    );

    auto* arity_array = futhark_new_i32_1d(ctx.get(), grammar::arities, grammar::NUM_PRODUCTIONS);
    auto* input_array = futhark_new_u8_1d(ctx.get(), reinterpret_cast<const uint8_t*>(input.data()), input.size());

    futhark_u8_1d* nodes = nullptr;
    futhark_i32_1d* parents = nullptr;

    int err = 0;
    if (lex_table && sct && pt && arity_array && input_array) {
        err = futhark_entry_main(
            ctx.get(),
            &nodes,
            &parents,
            input_array,
            lex_table,
            sct,
            pt,
            arity_array
        );

        if (err)
            report_futhark_error(ctx, "Main kernel failed");

        download_and_parse_tree(ctx, nodes, parents);
    } else {
        fmt::print(std::cerr, "Error: Failed to upload required data\n");
    }

    if (nodes)
        futhark_free_u8_1d(ctx.get(), nodes);

    if (parents)
        futhark_free_i32_1d(ctx.get(), parents);

    if (lex_table)
        futhark_free_opaque_lex_table(ctx.get(), lex_table);

    if (sct)
        futhark_free_opaque_stack_change_table(ctx.get(), sct);

    if (pt)
        futhark_free_opaque_parse_table(ctx.get(), pt);

    if (arity_array)
        futhark_free_i32_1d(ctx.get(), arity_array);

    if (input_array)
        futhark_free_u8_1d(ctx.get(), input_array);

    if (!err && opts.profile) {
        auto report = MallocPtr<char>(futhark_context_report(ctx.get()));
        fmt::print("Profile report:\n{}", report);
    }

    return !err ? EXIT_SUCCESS : EXIT_FAILURE;
}