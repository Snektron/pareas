#include "codegen/token.hpp"

#include <iostream>

const char* TOKEN_NAMES[] = {
    "EOF",

    "+",
    "-",
    "*",
    "/",
    "%",
    "&",
    "|",
    "^",

    "=",

    "==",
    "<",
    ">",
    "<=",
    ">=",

    "&&",
    "||",

    "(",
    ")",
    "{",
    "}",
    ",",
    ";",
    "@",
    "<-",

    "if",
    "else",
    "while",
    "int",
    "float",

    "integer",
    "identifier"
};

Token::Token(TokenType type) : type(type) {}
Token::Token(TokenType type, uint32_t integer) : type(type), integer(integer) {}
Token::Token(TokenType type, const std::string& str) : type(type), str(str) {}

std::ostream& operator<<(std::ostream& os, const TokenType& token_type) {
    os << TOKEN_NAMES[static_cast<size_t>(token_type)];
    return os;
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.type;

    switch(token.type) {
        case TokenType::INTEGER:
            os << " (" << token.integer << ")";
            break;
        case TokenType::ID:
            os << " (" << token.str << ")";
            break;
        default:
            break;
    }
    return os;
}