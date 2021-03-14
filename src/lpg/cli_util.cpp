#include "pareas/lpg/cli_util.hpp"

#include <iostream>
#include <iterator>
#include <fstream>

namespace pareas {
    std::optional<std::string> read_input(const char* filename) {
        std::string input;
        if (std::string_view(filename) == "-") {
            input.assign(
                std::istreambuf_iterator<char>(std::cin),
                std::istreambuf_iterator<char>()
            );
        } else {
            auto in = std::ifstream(filename, std::ios::binary);
            if (!in) {
                return std::nullopt;
            }
            input.assign(
                std::istreambuf_iterator<char>(in),
                std::istreambuf_iterator<char>()
            );
        }
        return input;
    }
}
