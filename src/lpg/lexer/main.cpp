#include "pareas/lpg/lexer/lexer_parser.hpp"
#include "pareas/lpg/lexer/fsa.hpp"
#include "pareas/lpg/lexer/parallel_lexer.hpp"
#include "pareas/lpg/lexer/render.hpp"
#include "pareas/lpg/error_reporter.hpp"
#include "pareas/lpg/parser.hpp"
#include "pareas/lpg/cli_util.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <optional>
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace {
    struct Options {
        const char* input_path;
        const char* code_output_path;
        const char* initial_state_output_path;
        const char* merge_table_output_path;
        const char* final_state_output_path;
        bool help;
    };

    void print_usage(const char* progname) {
        fmt::print(
            "Usage: {} [options...] <input path>\n"
            "Available options:\n"
            "--code-out <path>           Write generated source code to <path>.\n"
            "--initial-state-out <path>  Write initial state dataset to <path>.\n"
            "--merge-table-out <path>    Write merge table dataset to <path>.\n"
            "--final-state-out <path>    Write final state dataset to <path>.\n"
            "-h --help                   Show this message and exit\n"
            "\n"
            "When <input path> is '-', standard input is used.\n",
            progname
        );
    }

    bool parse_options(Options* opts, int argc, const char* argv[]) {
        *opts = {
            .input_path = nullptr,
            .code_output_path = nullptr,
            .initial_state_output_path = nullptr,
            .merge_table_output_path = nullptr,
            .final_state_output_path = nullptr,
            .help = false,
        };

        for (int i = 1; i < argc; ++i) {
            auto arg = std::string_view(argv[i]);

            const char** path_ptr = nullptr;

            if (arg == "-h" || arg == "--help") {
                opts->help = true;
            } else if (arg == "--code-out") {
                path_ptr = &opts->code_output_path;
            } else if (arg == "--initial-state-out") {
                path_ptr = &opts->initial_state_output_path;
            } else if (arg == "--merge-table-out") {
                path_ptr = &opts->merge_table_output_path;
            } else if (arg == "--final-state-out") {
                path_ptr = &opts->final_state_output_path;
            } else if (!opts->input_path) {
                opts->input_path = argv[i];
            } else {
                fmt::print(std::cerr, "Error: Unknown option {}\n", arg);
                return false;
            }

            if (!path_ptr)
                continue;

            if (++i >= argc) {
                fmt::print(std::cerr, "Error: Expected argument <path> to option {}\n", arg);
                return false;
            }

            *path_ptr = argv[i];
        }

        if (opts->help)
            return true;

        if (!opts->input_path) {
            fmt::print(std::cerr, "Error: Missing required argument <input path>\n");
            return false;
        } else if (!opts->code_output_path) {
            fmt::print(std::cerr, "Error: Missing required argument --code-out\n");
            return false;
        } else if (!opts->initial_state_output_path) {
            fmt::print(std::cerr, "Error: Missing required argument --initial-state-out\n");
            return false;
        } else if (!opts->merge_table_output_path) {
            fmt::print(std::cerr, "Error: Missing required argument --merge-table-out\n");
            return false;
        } else if (!opts->final_state_output_path) {
            fmt::print(std::cerr, "Error: Missing required argument --final-state-out\n");
            return false;
        }

        return true;
    }

    std::optional<std::ofstream> open_output(const char* filename) {
        auto out = std::ofstream(filename, std::ios::binary);
        if (!out) {
            fmt::print(std::cerr, "Error: Failed to open output path '{}'\n", filename);
            return std::nullopt;
        }
        return out;
    }
}

int main_(int argc, const char* argv[]) {
    Options opts;
    if (!parse_options(&opts, argc, argv)) {
        fmt::print(std::cerr, "See '{} --help' for usage\n", argv[0]);
        return EXIT_FAILURE;
    } else if (opts.help) {
        print_usage(argv[0]);
    }

    std::string input;
    if (auto maybe_input = pareas::read_input(opts.input_path)) {
        input = std::move(maybe_input.value());
    } else {
        fmt::print(std::cerr, "Error: Failed to open input path '{}'\n", opts.input_path);
        return EXIT_FAILURE;
    }

    auto er = pareas::ErrorReporter(input, std::clog);
    auto parser = pareas::Parser(&er, input);
    auto lexer_parser = pareas::lexer::LexerParser(&parser);
    auto grammar = lexer_parser.parse();
    auto parallel_lexer = pareas::lexer::ParallelLexer(&grammar);
    // auto renderer = pareas::lexer::LexerRenderer(&grammar, &parallel_lexer);

    // if (auto out = open_output(opts.code_output_path)) {
    //     renderer.render_code(out.value());
    // } else {
    //     return EXIT_FAILURE;
    // }

    // if (auto out = open_output(opts.initial_state_output_path)) {
    //     renderer.render_initial_state_dataset(out.value());
    // } else {
    //     return EXIT_FAILURE;
    // }

    // if (auto out = open_output(opts.merge_table_output_path)) {
    //     renderer.render_merge_table_dataset(out.value());
    // } else {
    //     return EXIT_FAILURE;
    // }

    // if (auto out = open_output(opts.final_state_output_path)) {
    //     renderer.render_final_state_dataset(out.value());
    // } else {
    //     return EXIT_FAILURE;
    // }

    return EXIT_SUCCESS;
}
