#ifndef _PAREAS_CODEGEN_EXCEPTION_HPP
#define _PAREAS_CODEGEN_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include <sstream>

template <typename... Args>
std::string make_error_msg(const Args&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return ss.str();
}

class ParseException : public std::runtime_error {
    public:
        template <typename... Args>
        ParseException(const Args&... args) : std::runtime_error(make_error_msg(args...)) {}
        virtual ~ParseException() = default;
};

#endif
