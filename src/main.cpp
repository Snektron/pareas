#include <iostream>
#include <fstream>

#include "codegen/lexer.hpp"
#include "codegen/exception.hpp"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "No input file given" << std::endl;
        return 1;
    }

    try {
        std::ifstream input(argv[1]);
        Lexer lexer(input);

        Token token = lexer.lex();
        while(token.type != TokenType::END_OF_FILE) {
            std::cout << token << std::endl;

            token = lexer.lex();
        }
    }
    catch(const ParseException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}