#ifndef _PAREAS_CODEGEN_LEXER_HPP
#define _PAREAS_CODEGEN_LEXER_HPP

#include <iosfwd>
#include <stack>

#include "codegen/token.hpp"

class Lexer {
    private:
        std::istream& input;
        std::stack<char> char_stack;
        std::stack<Token> token_stack;
        size_t linenr = 1;

        bool isWhitespace(char c);
        bool isIdChar(char c);
        char read();
        void unread(char c);

        Token next_number();
        Token next_id();
        Token next_token();
    public:
        Lexer(std::istream&);

        Token lex();
        void unlex(const Token&);
        Token lookahead();

        inline size_t line() {
            return this->linenr;
        }
};

#endif
