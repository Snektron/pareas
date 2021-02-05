#ifndef _PAREAS_CODEGEN_TOKEN_HPP
#define _PAREAS_CODEGEN_TOKEN_HPP

#include <iosfwd>
#include <cstdint>
#include <string>

enum class TokenType {
    END_OF_FILE,

    PLUS,
    MIN,
    MUL,
    DIV,
    MOD,
    BITAND,
    BITOR,
    BITXOR,

    ASSIGN,

    EQ,
    LESS,
    GREATER,
    LESSEQ,
    GREATEQ,

    LAND,
    LOR,

    OPEN_PAR,
    CLOSE_PAR,
    OPEN_CB,
    CLOSE_CB,
    COMMA,
    SEMICOLON,

    IF,
    ELSE,
    WHILE,
    INT,
    FLOAT,

    INTEGER,
    ID
};

struct Token {
    TokenType type;
    uint32_t integer;
    std::string str;

    Token(TokenType);
    Token(TokenType, uint32_t);
    Token(TokenType, const std::string&);
};

std::ostream& operator<<(std::ostream&, const Token&);

#endif
