#include <futhark-generated.h>
#include <iostream>
#include <string_view>
#include <cstdlib>

struct Options {
    const char* input_path;
    const char* output_path;
    bool help;
    bool verbose;
    bool debug;
};

void print_usage(const char* progname) {
    std::cerr << "Usage: " << progname << " [options...] <input path>\n"
        "Available options:\n"
        "-o --output <output path>   Write the output to <output path>. (default: b.out)\n"
        "-h --help                   Show this message and exit.\n"
        "-v --verbose                Enable Futhark logging.\n"
        "-d --debug                  Enable Futhark debug logging.\n"
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
    };

    for (int i = 1; i < argc; ++i) {
        auto arg = std::string_view(argv[i]);

        if (arg == "-o" || arg == "--output") {
            if (++i >= argc) {
                std::cerr << "Error: Expected argument <output path> to option '" << arg << "'" << std::endl;
                return false;
            }
            opts->output_path = argv[i];
        } else if (arg == "-h" || arg == "--help") {
            opts->help = true;
        } else if (arg == "-v" || arg == "--verbose") {
            opts->verbose = true;
        } else if (arg == "--debug") {
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

    return true;
}

int main(int argc, const char* argv[]) {
    Options opts;
    if (!parse_options(&opts, argc, argv)) {
        std::cerr << "See " << argv[0] << " --help for usage" << std::endl;
        return EXIT_FAILURE;
    } else if (opts.help) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
