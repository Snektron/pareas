#include "json_futhark_generated.h"
#include "json_grammar.hpp"

#include "pareas/json/futhark_interop.hpp"
#include "pareas/profiler/profiler.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>

#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <charconv>
#include <cstdlib>
#include <cstdio>

// This file is mostly just copied from src/compiler/main.cpp

struct Options {
    const char* input_path;
    bool help;
    bool futhark_verbose;
    bool futhark_debug;
    bool futhark_debug_extra;
    bool dump_dot;
    bool verbose_tree;

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
        "--futhark-debug-extra       Futhark debug logging with extra information.\n"
        "                            Not compatible with --futhark-debug.\n"
        "--dump-dot                  Dump JSON tree as dot graph. Disables profiling.\n"
        "--verbose-tree              Print some information about the document tree.\n"
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
        .futhark_debug_extra = false,
        .dump_dot = false,
        .verbose_tree = false,
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
        } else if (arg == "--futhark-verbose") {
            opts->futhark_verbose = true;
        } else if (arg == "--futhark-debug") {
            opts->futhark_debug = true;
        } else if (arg == "--futhark-debug-extra") {
            opts->futhark_debug_extra = true;
        } else if (arg == "--dump-dot") {
            opts->dump_dot = true;
        } else if (arg == "--verbose-tree") {
            opts->verbose_tree = true;
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

futhark::UniqueLexTable upload_lex_table(futhark_context* ctx) {
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
T upload_strtab(futhark_context* ctx, const json::StrTab<U>& strtab, F upload_fn) {
    static_assert(sizeof(U) == sizeof(uint8_t));

    auto table = futhark::UniqueArray<uint8_t, 1>(
        ctx,
        reinterpret_cast<const uint8_t*>(strtab.table),
        strtab.n
    );

    auto offsets = futhark::UniqueArray<int32_t, 2>(ctx, strtab.offsets, json::NUM_TOKENS, json::NUM_TOKENS);
    auto lengths = futhark::UniqueArray<int32_t, 2>(ctx, strtab.lengths, json::NUM_TOKENS, json::NUM_TOKENS);

    auto tab = T(ctx);

    int err = upload_fn(ctx, &tab, table.get(), offsets.get(), lengths.get());
    if (err != 0)
        throw futhark::Error(ctx);

    return tab;
}

struct JsonTree {
    size_t num_nodes;
    std::unique_ptr<json::Production[]> node_types;
    std::unique_ptr<int32_t[]> parents;
};

void dump_dot(const JsonTree& j, std::ostream& os) {
    fmt::print(os, "digraph json {{\n");

    for (size_t i = 0; i < j.num_nodes; ++i) {
        auto prod = j.node_types[i];
        auto parent = j.parents[i];
        auto* name = json::production_name(prod);

        if (parent == i)
            continue;

        fmt::print(os, "node{} [label=\"{}\nindex={}\"]\n", i, name, i);

        if (parent >= 0) {
            fmt::print(os, "node{} -> node{};\n", parent, i);
        } else {
            fmt::print(os, "start{0} [style=invis];\nstart{0} -> node{0};\n", i);
        }
    }

    fmt::print(os, "}}\n");
}

JsonTree parse(futhark_context* ctx, const std::string& input, pareas::Profiler& p, std::FILE* debug_log) {
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
        json::stack_change_table,
        futhark_entry_mk_stack_change_table
    );

    auto pt = upload_strtab<futhark::UniqueParseTable>(
        ctx,
        json::parse_table,
        futhark_entry_mk_parse_table
    );

    auto arity_array = futhark::UniqueArray<int32_t, 1>(ctx, json::arities, json::NUM_PRODUCTIONS);
    p.end("table");

    p.begin();
    auto input_array = futhark::UniqueArray<uint8_t, 1>(ctx, reinterpret_cast<const uint8_t*>(input.data()), input.size());
    p.end("input");
    p.end("upload");

    p.begin();

    debug_log_region("tokenize");
    auto tokens = futhark::UniqueArray<uint8_t, 1>(ctx);
    p.measure("tokenize", [&]{
        int err = futhark_entry_json_lex(ctx, &tokens, input_array, lex_table);
        if (err)
            throw futhark::Error(ctx);
    });

    fmt::print("Num tokens: {}\n", tokens.shape()[0]);

    debug_log_region("parse");
    auto node_types = futhark::UniqueArray<uint8_t, 1>(ctx);
    p.measure("parse", [&]{
        bool valid = false;
        int err = futhark_entry_json_parse(ctx, &valid, &node_types, tokens, sct, pt);
        if (err)
            throw futhark::Error(ctx);
        if (!valid)
            throw std::runtime_error("Parse error");
    });

    debug_log_region("build parse tree");
    auto parents = futhark::UniqueArray<int32_t, 1>(ctx);
    p.measure("build parse tree", [&]{
        int err = futhark_entry_json_build_parse_tree(ctx, &parents, node_types, arity_array);
        if (err)
            throw futhark::Error(ctx);
    });

    debug_log_region("restructure");
    p.measure("restructure", [&]{
        auto old_node_types = std::move(node_types);
        auto old_parents = std::move(parents);
        int err = futhark_entry_json_restructure(ctx, &node_types, &parents, old_node_types, old_parents);
        if (err)
            throw futhark::Error(ctx);
    });

    debug_log_region("validate");
    p.measure("validate", [&]{
        bool valid;
        int err = futhark_entry_json_validate(ctx, &valid, node_types, parents);
        if (err)
            throw futhark::Error(ctx);
        if (!valid)
            throw std::runtime_error("Invalid structure");
    });

    p.end("json");

    size_t num_nodes = node_types.shape()[0];

    auto ast = JsonTree {
        .num_nodes = num_nodes,
        .node_types = std::make_unique<json::Production[]>(num_nodes),
        .parents = std::make_unique<int32_t[]>(num_nodes),
    };

    int err = futhark_values_u8_1d(
        ctx,
        node_types,
        reinterpret_cast<std::underlying_type_t<json::Production>*>(ast.node_types.get())
    );

    err |= futhark_values_i32_1d(ctx, parents, ast.parents.get());

    if (err)
        throw futhark::Error(ctx);

    return ast;
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
    futhark_context_config_set_debugging(config.get(), opts.futhark_debug || opts.futhark_debug_extra);

    #if defined(FUTHARK_BACKEND_multicore)
        futhark_context_config_set_num_threads(config.get(), opts.threads);
    #elif defined(FUTHARK_BACKEND_opencl) || defined(FUTHARK_BACKEND_cuda)
        if (opts.device_name) {
            futhark_context_config_set_device(config.get(), opts.device_name);
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
        auto ast = parse(ctx.get(), input, p, opts.futhark_debug_extra ? stderr : nullptr);

        if (opts.dump_dot)
            dump_dot(ast, std::cout);
        else
            p.dump(std::cout);

        if (opts.verbose_tree) {
            fmt::print("Nodes: {}\n", ast.num_nodes);
        }

        if (opts.futhark_profile) {
            auto report = MallocPtr<char>(futhark_context_report(ctx.get()));
            fmt::print(std::cerr, "Profile report:\n{}", report);
        }
    } catch (const std::runtime_error& err) {
        fmt::print(std::cerr, "Error: {}\n", err.what());
        return EXIT_FAILURE;
    } catch (const futhark::Error& err) {
        fmt::print(std::cerr, "Futhark error: {}\n", err.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
