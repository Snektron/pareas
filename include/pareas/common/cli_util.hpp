#ifndef _PAREAS_COMMON_CLI_UTIL_HPP
#define _PAREAS_COMMON_CLI_UTIL_HPP

#include <optional>
#include <string>

namespace pareas {
    std::optional<std::string> read_input(const char* filename);
}

#endif
