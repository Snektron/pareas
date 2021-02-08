#include <iostream>
#include <fstream>
#include <memory>

#include "codegen/lexer.hpp"
#include "codegen/parser.hpp"
#include "codegen/exception.hpp"
#include "codegen/astnode.hpp"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "No input file given" << std::endl;
        return 1;
    }

    try {
        std::ifstream input(argv[1]);
        Lexer lexer(input);
        Parser parser(lexer);

        std::unique_ptr<ASTNode> node(parser.parse());
        std::cout << *node << std::endl;

        node->resolveType();
        std::cout << *node << std::endl;
    }
    catch(const ParseException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}