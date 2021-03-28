#include "pareas/lpg/error_reporter.hpp"
#include "pareas/lpg/parser.hpp"
#include "pareas/lpg/cli_util.hpp"
#include "pareas/lpg/token_mapping.hpp"
#include "pareas/lpg/renderer.hpp"
#include "pareas/lpg/parser/grammar.hpp"
#include "pareas/lpg/parser/grammar_parser.hpp"
#include "pareas/lpg/parser/terminal_set_functions.hpp"
#include "pareas/lpg/parser/ll/generator.hpp"
#include "pareas/lpg/parser/llp/generator.hpp"
#include "pareas/lpg/parser/llp/render.hpp"
#include "pareas/lpg/lexer/lexer_parser.hpp"
#include "pareas/lpg/lexer/parallel_lexer.hpp"
#include "pareas/lpg/lexer/render.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <fstream>
#include <string_view>
#include <string>
#include <iterator>
#include <stdexcept>
#include <optional>
#include <cstdlib>
#include <cassert>

namespace {
    using namespace pareas;

    struct Options {
        const char* parser_src;
        const char* lexer_src;
        const char* output;
        const char* namesp;
        bool verbose_lexer;
        bool verbose_grammar;
        bool verbose_sets;
        bool verbose_psls;
        bool verbose_ll;
        bool verbose_llp;
        bool help;
    };

    void print_usage(const char* progname) {
        fmt::print(
            "Usage: {} [options...]\n"
            "\n"
            "Available options:\n"
            "--parser <grammar.llpg>     Generate a parser from <grammar.llpg>.\n"
            "--lexer <lexer.lex>         Generate a lexer from <lexer.lex>.\n"
            "-o --output <path>          Basename of generated output files.\n"
            "--namespace <namespace>     Emit c++ definitions under <namespace>\n"
            "--verbose-lexer             Dump sizes of lexer tables.\n"
            "--verbose-grammar           Dump parsed grammar to stderr.\n"
            "--verbose-sets              Dump first/last/follow/before sets to stderr.\n"
            "--verbose-psls              Dump PSLS as CSV to stderr.\n"
            "--verbose-ll                Dump LL table as CSV to stderr.\n"
            "--verbose-llp               Dump LLP table as CSV to stderr.\n"
            "-h --help                   Show this message and exit.\n"
            "\n"
            "Either or both of --parser and --lexer are required, as well as --output.\n",
            progname
        );
    }

    bool parse_options(Options& opts, int argc, char* argv[]) {
        opts = {
            .parser_src = nullptr,
            .lexer_src = nullptr,
            .output = nullptr,
            .namesp = nullptr,
            .verbose_lexer = false,
            .verbose_grammar = false,
            .verbose_sets = false,
            .verbose_psls = false,
            .verbose_ll = false,
            .verbose_llp = false,
            .help = false,
        };

        for (int i = 1; i < argc; ++i) {
            auto arg = std::string_view(argv[i]);

            const char** ptr = nullptr;
            const char* argname = nullptr;

            if (arg == "--parser") {
                ptr = &opts.parser_src;
                argname = "grammar.llpg";
            } else if (arg == "--lexer") {
                ptr = &opts.lexer_src;
                argname = "lexer.lex";
            } else if (arg == "-o" || arg == "--output") {
                ptr = &opts.output;
                argname = "path";
            } else if (arg == "--namespace") {
                ptr = &opts.namesp;
                argname = "namespace";
            } else if (arg == "--verbose-lexer") {
                opts.verbose_lexer = true;
            } else if (arg == "--verbose-grammar") {
                opts.verbose_grammar = true;
            } else if (arg == "--verbose-sets") {
                opts.verbose_sets = true;
            } else if (arg == "--verbose-psls") {
                opts.verbose_psls = true;
            } else if (arg == "--verbose-ll") {
                opts.verbose_ll = true;
            } else if (arg == "--verbose-llp") {
                opts.verbose_llp = true;
            } else if (arg == "--help" || arg == "-h") {
                opts.help = true;
            } else {
                fmt::print("Error: Unknown option {}\n", arg);
                return false;
            }

            if (!ptr)
                continue;

            if (++i >= argc) {
                fmt::print(std::cerr, "Error: Expected argument <{}> to option {}\n", argname, arg);
                return false;
            }
            *ptr = argv[i];
        }

        if (opts.help)
            return true;

        if (!opts.parser_src && !opts.lexer_src) {
            fmt::print(std::cerr, "Error: Missing either or both of --parser or --lexer\n");
            return false;
        }

        if (!opts.output) {
            fmt::print(std::cerr, "Error: Missing required argument --output\n");
            return false;
        }

        if (!opts.namesp) {
            fmt::print(std::cerr, "Error: Missing required argument --namespace\n");
            return false;
        }

        return true;
    }

    std::optional<std::string> read_input(const char* filename) {
        auto in = std::ifstream(filename, std::ios::binary);
        if (!in) {
            fmt::print(std::cerr, "Error: Failed to open input file '{}'\n", filename);
            return std::nullopt;
        }

        return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }

    struct LexerGeneration {
        lexer::LexicalGrammar grammar;
        lexer::ParallelLexer parallel_lexer;
    };

    std::optional<LexerGeneration> generate_lexer(const Options& opts, TokenMapping& tm) {
        std::string input;
        if (auto maybe_input = read_input(opts.lexer_src)) {
            input = std::move(maybe_input.value());
        } else {
            return std::nullopt;
        }

        try {
            auto er = ErrorReporter(input, std::clog);
            auto parser = Parser(&er, input);
            auto lexer_parser = lexer::LexerParser(&parser);

            auto g = lexer_parser.parse();
            g.validate(er);

            auto parallel_lexer = lexer::ParallelLexer(&g);

            if (opts.verbose_lexer) {
                parallel_lexer.dump_sizes(std::cout);
            }

            g.add_tokens(tm);

            return {{std::move(g), std::move(parallel_lexer)}};
        } catch (const std::runtime_error& e) {
            fmt::print(std::cerr, "Failed to generate lexer: {}\n", e.what());
            return std::nullopt;
        }
    }

    struct ParserGeneration {
        parser::Grammar grammar;
        parser::llp::ParsingTable llp_table;
    };

    std::optional<ParserGeneration> generate_parser(const Options& opts, TokenMapping& tm, bool derive_tokens) {
        std::string input;
        if (auto maybe_input = read_input(opts.parser_src)) {
            input = std::move(maybe_input.value());
        } else {
            return std::nullopt;
        }

        try {
            auto er = ErrorReporter(input, std::clog);
            auto parser = Parser(&er, input);
            auto grammar_parser = parser::GrammarParser(&parser);
            auto g = grammar_parser.parse();

            if (opts.verbose_grammar)
                g.dump(std::clog);

            auto tsf = parser::TerminalSetFunctions(g);
            if (opts.verbose_sets)
                tsf.dump(std::clog);

            auto gen = parser::llp::Generator(&er, &g, &tsf);

            auto psls_table = gen.build_psls_table();
            if (opts.verbose_psls)
                psls_table.dump_csv(std::clog);

            auto ll_table = parser::ll::Generator(&er, &g, &tsf).build_parsing_table();
            if (opts.verbose_ll)
                ll_table.dump_csv(std::clog);

            auto llp_table = gen.build_parsing_table(ll_table, psls_table);
            if (opts.verbose_llp)
                llp_table.dump_csv(std::clog);

            // If also generating a lexer, first check whether all the tokens currently in the
            // token mapping also appear in the parse grammar.
            if (!derive_tokens) {
                g.link_tokens(er, tm);
            }

            // Now add the required special start- and end-of-index tokens.
            // TODO: This can be improved by just adding the required tokens and not
            // iterating over the entire grammar again.
            g.add_tokens(tm);

            return {{std::move(g), std::move(llp_table)}};
        } catch (const std::runtime_error& e) {
            fmt::print(std::cerr, "Failed to generate parser: {}\n", e.what());
            return std::nullopt;
        }
    }
}

int main(int argc, char* argv[]) {
    Options opts;
    if (!parse_options(opts, argc, argv)) {
        fmt::print(std::cerr, "See '{} --help' for usage\n", argv[0]);
        return EXIT_FAILURE;
    } else if (opts.help) {
        print_usage(argv[0]);
    }

    auto tm = TokenMapping();

    auto lexer = opts.lexer_src ? generate_lexer(opts, tm) : std::nullopt;
    auto parser = opts.parser_src ? generate_parser(opts, tm, !lexer.has_value()) : std::nullopt;

    // Only do this check here so we can report errors for both parser and lexer construction.
    if ((opts.lexer_src && !lexer.has_value()) || (opts.parser_src && !parser.has_value()))
        return EXIT_FAILURE;

    try {
        auto renderer = pareas::Renderer(opts.namesp, opts.output);

        tm.render(renderer);

        if (lexer.has_value()) {
            auto lr = pareas::lexer::LexerRenderer(&renderer, &tm, &lexer->parallel_lexer);
            lr.render();
        }

        if (parser.has_value()) {
            auto pr = pareas::parser::llp::ParserRenderer(&renderer, &tm, &parser->grammar, &parser->llp_table);
            pr.render();
        }

        renderer.finalize();
    } catch (const RenderError& e) {
        fmt::print("Error: {}\n", e.what());
    }

    return EXIT_SUCCESS;
}
