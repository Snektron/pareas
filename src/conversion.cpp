#include <iostream>
#include <string_view>
#include <memory>
#include <charconv>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <bitset>

#include "codegen/lexer.hpp"
#include "codegen/parser.hpp"
#include "codegen/astnode.hpp"
#include "codegen/exception.hpp"
#include "codegen/depthtree.hpp"
#include "codegen/symtab.hpp"
#include "codegen/treeproperties.hpp"

#include "profiler/profiler.hpp"

#include "converter/converter.hpp"


int main(int argc, const char* argv[]) {
    if(argc < 3) {
        std::cerr << "Not enough arguments given" << std::endl;
        return EXIT_FAILURE;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];

    auto p = pareas::Profiler(100);

    try {
        std::ifstream input(input_file);
        if(!input) {
            std::cerr << "Failed to open file " << input_file << std::endl;
            return EXIT_FAILURE;
        }
        std::ofstream output(output_file);
        if(!output) {
            std::cerr << "Failed to create file " << output_file << std::endl;
            return EXIT_FAILURE;
        }

        p.begin();

        //Stage 0, CPU setup
        p.begin();
        Lexer lexer(input);
        SymbolTable symtab;
        Parser parser(lexer, symtab);

        std::unique_ptr<ASTNode> node(parser.parse());
        node->resolveType();

        p.end("Parsing");
        p.begin();

        pareas::SourceConverter converter(output, &symtab);
        converter.convert(node.get());

        p.end("Convert");

        TreeProperties props(node.get());
        std::cout << "Number of nodes: " << props.getNodeCount() << std::endl;
        std::cout << "Tree width: " << props.getWidth() << std::endl;
        std::cout << "Tree height: " << props.getDepth() << std::endl;
        std::cout << "Num functions: " << props.getFunctions() << std::endl;
        std::cout << "Max function length: " << props.getMaxFuncLen() << std::endl;

        p.end("Total");
        p.dump(std::cout);
    }
    catch(const ParseException& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}