#include "futhark_generated.h"
#include "pareas_grammar.hpp"

#include "pareas/compiler/futhark_interop.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/chrono.h>

#include <iostream>
#include <fstream>
#include <string_view>
#include <memory>
#include <chrono>
#include <charconv>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>

// Keep in sync with src/compiler/frontend.fut
enum class Status : uint8_t {
    OK = 0,
    PARSE_ERROR = 1,
    STRAY_ELSE_ERROR = 2,
    INVALID_DECL = 3,
    INVALID_PARAMS = 4,
    INVALID_ASSIGN = 5,
    INVALID_FN_PROTO = 6,
    DUPLICATE_FN_OR_INVALID_CALL = 7,
    INVALID_VARIABLE = 8,
    INVALID_ARG_COUNT = 9,
    TYPE_ERROR = 10,
    INVALID_RETURN = 11,
    MISSING_RETURN = 12,
};

const char* status_name(Status s) {
    switch (s) {
        case Status::OK: return "ok";
        case Status::PARSE_ERROR: return "parse error";
        case Status::STRAY_ELSE_ERROR: return "stray else/elif";
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

// Keep in sync with src/compiler/datanode_types.fut
enum class DataType : uint8_t {
    INVALID = 0,
    VOID = 1,
    INT = 2,
    FLOAT = 3,
    INT_REF = 4,
    FLOAT_REF = 5,
};

const char* data_type_name(DataType dt) {
    switch (dt) {
        case DataType::INVALID: return "invalid";
        case DataType::VOID: return "void";
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::INT_REF: return "int ref";
        case DataType::FLOAT_REF: return "float ref";
    }
}

struct Options {
    const char* input_path;
    const char* output_path;
    bool help;
    bool verbose;
    bool debug;
    bool dump_dot;
    bool lexer_match;

    // Options available for the multicore backend
    int threads;

    // Options abailable for the OpenCL and CUDA backends
    const char* device_name;
    bool profile;
};

void print_usage(char* progname) {
    fmt::print(
        "Usage: {} [options...] <input path>\n"
        "Available options:\n"
        "-o --output <output path>   Write the output to <output path>. (default: b.out)\n"
        "-h --help                   Show this message and exit.\n"
        "-v --verbose                Enable Futhark logging.\n"
        "-d --debug                  Enable Futhark debug logging.\n"
        "--dump-dot                  Dump tree as dot graph.\n"
        "--lexer-match               Check whether lexical analysis passes.\n"
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

bool parse_options(Options* opts, int argc, char* argv[]) {
    *opts = {
        .input_path = nullptr,
        .output_path = "b.out",
        .help = false,
        .verbose = false,
        .debug = false,
        .dump_dot = false,
        .lexer_match = false,
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
        } else if (arg == "--dump-dot") {
            opts->dump_dot = true;
        } else if (arg == "--lexer-match") {
            opts->lexer_match = true;
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

futhark::UniqueLexTable upload_lex_table(futhark::Context& ctx) {
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
T upload_strtab(futhark::Context& ctx, const grammar::StrTab<U>& strtab, F upload_fn) {
    static_assert(sizeof(U) == sizeof(uint8_t));

    auto table = futhark::UniqueArray<uint8_t, 1>(
        ctx,
        reinterpret_cast<const uint8_t*>(strtab.table),
        strtab.n
    );

    auto offsets = futhark::UniqueArray<int32_t, 2>(ctx, strtab.offsets, grammar::NUM_TOKENS, grammar::NUM_TOKENS);
    auto lengths = futhark::UniqueArray<int32_t, 2>(ctx, strtab.lengths, grammar::NUM_TOKENS, grammar::NUM_TOKENS);

    auto tab = T(ctx);

    int err = upload_fn(ctx.get(), &tab, table.get(), offsets.get(), lengths.get());
    if (err != 0)
        throw futhark::Error(ctx);

    return tab;
}

void render_tree(
    size_t n,
    const grammar::Production* node_types,
    const int32_t* parents,
    const uint32_t* data,
    const DataType* data_types,
    const int32_t* depths,
    const int32_t* child_indexes,
    const int32_t* fn_tab
) {
    fmt::print("digraph prog {{\n");

    for (size_t i = 0; i < n; ++i) {
        auto prod = node_types[i];
        auto parent = parents[i];
        auto* name = grammar::production_name(prod);

        if (parent != i) {
            fmt::print("node{} [label=\"{}\nindex={}\ndepth={}\nchild index={}", i, name, i, depths[i], child_indexes[i]);

            switch (prod) {
                case grammar::Production::ATOM_NAME:
                case grammar::Production::ATOM_DECL:
                case grammar::Production::ATOM_DECL_EXPLICIT:
                    fmt::print("\\n(offset={})", data[i]);
                    break;
                case grammar::Production::FN_DECL:
                    fmt::print("\\n(num locals={})", fn_tab[data[i]]);
                case grammar::Production::ATOM_FN_CALL:
                    fmt::print("\\n(fn id={})", data[i]);
                    break;
                case grammar::Production::PARAM:
                case grammar::Production::ARG:
                    fmt::print("\\n(arg id={})", data[i]);
                    break;
                case grammar::Production::ATOM_INT:
                    fmt::print("\\n(value={})", data[i]);
                    break;
                case grammar::Production::ATOM_FLOAT:
                    fmt::print("\\n(value={})", *reinterpret_cast<const float*>(&data[i]));
                    break;
                default:
                    if (data[i] != 0) {
                        fmt::print("\\n(junk={})", data[i]);
                    }
            }

            fmt::print("\\n[{}]", data_type_name(data_types[i]));
            fmt::print("\"]\n");

            if (parent >= 0) {
                fmt::print("node{} -> node{};\n", parent, i);
            } else {
                fmt::print("start{0} [style=invis];\nstart{0} -> node{0};\n", i);
            }
        }
    }

    fmt::print("}}\n");
}

void download_and_render_tree(
    futhark::Context& ctx,
    futhark::UniqueArray<uint8_t, 1>& node_types,
    futhark::UniqueArray<int32_t, 1>& parents,
    futhark::UniqueArray<uint32_t, 1>& data,
    futhark::UniqueArray<uint8_t, 1>& data_types,
    futhark::UniqueArray<int32_t, 1>& depths,
    futhark::UniqueArray<int32_t, 1>& child_indexes,
    futhark::UniqueArray<int32_t, 1>& fn_tab
) {
    int64_t n = node_types.shape()[0];

    auto host_node_types = node_types.download();
    auto host_parents = parents.download();
    auto host_data = data.download();
    auto host_data_types = data_types.download();
    auto host_depths = depths.download();
    auto host_child_indexes = child_indexes.download();
    auto host_fn_tab = fn_tab.download();

    if (futhark_context_sync(ctx.get()))
        throw futhark::Error(ctx);

    render_tree(
        n,
        reinterpret_cast<grammar::Production*>(host_node_types.data()),
        host_parents.data(),
        host_data.data(),
        reinterpret_cast<DataType*>(host_data_types.data()),
        host_depths.data(),
        host_child_indexes.data(),
        host_fn_tab.data()
    );
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

    auto start = std::chrono::high_resolution_clock::now();

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
    auto input_array = futhark::UniqueArray<uint8_t, 1>(ctx, reinterpret_cast<const uint8_t*>(input.data()), input.size());

    if (futhark_context_sync(ctx.get()) != 0)
        throw futhark::Error(ctx);

    auto stop = std::chrono::high_resolution_clock::now();
    fmt::print(std::cerr, "Upload time: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

    if (opts.lexer_match) {
        start = std::chrono::high_resolution_clock::now();

        bool result;
        int err = futhark_entry_lex_match(
            ctx.get(),
            &result,
            input_array.get(),
            lex_table.get()
        );

        if (err != 0 || futhark_context_sync(ctx.get()) != 0)
            throw futhark::Error(ctx);

        stop = std::chrono::high_resolution_clock::now();
        fmt::print(std::cerr, "Main kernel runtime: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(stop - start));
        fmt::print(std::cerr, "Result: {}\n", result);


        if (opts.profile) {
            auto report = MallocPtr<char>(futhark_context_report(ctx.get()));
            fmt::print("Profile report:\n{}", report);
        }

        return EXIT_SUCCESS;
    }

    auto node_types = futhark::UniqueArray<uint8_t, 1>(ctx);
    auto parents = futhark::UniqueArray<int32_t, 1>(ctx);
    auto data = futhark::UniqueArray<uint32_t, 1>(ctx);
    auto data_types = futhark::UniqueArray<uint8_t, 1>(ctx);
    auto child_indexes = futhark::UniqueArray<int32_t, 1>(ctx);
    auto depths = futhark::UniqueArray<int32_t, 1>(ctx);
    auto fn_tab = futhark::UniqueArray<int32_t, 1>(ctx);
    Status status;

    start = std::chrono::high_resolution_clock::now();
    int err = futhark_entry_main(
        ctx.get(),
        reinterpret_cast<std::underlying_type_t<Status>*>(&status),
        &node_types,
        &parents,
        &data,
        &data_types,
        &depths,
        &child_indexes,
        &fn_tab,
        input_array.get(),
        lex_table.get(),
        sct.get(),
        pt.get(),
        arity_array.get()
    );

    if (err != 0)
        throw futhark::Error(ctx);

    if (futhark_context_sync(ctx.get()) != 0)
        throw futhark::Error(ctx);

    stop = std::chrono::high_resolution_clock::now();
    fmt::print(std::cerr, "Main kernel runtime: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(stop - start));

    if (status == Status::OK) {
        fmt::print(std::cerr, "{} nodes\n", node_types.shape()[0]);

        if (opts.dump_dot) {
            download_and_render_tree(
                ctx,
                node_types,
                parents,
                data,
                data_types,
                depths,
                child_indexes,
                fn_tab
            );
        }
    } else {
        fmt::print(std::cerr, "Error: {}\n", status_name(status));
        err = 1;
    }

    if (opts.profile) {
        auto report = MallocPtr<char>(futhark_context_report(ctx.get()));
        fmt::print("Profile report:\n{}", report);
    }

    if (futhark_context_sync(ctx.get()) != 0)
        throw futhark::Error(ctx);

    return EXIT_SUCCESS;
}
