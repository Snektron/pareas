#include "codegen/lexer.hpp"
#include "codegen/exception.hpp"

#include <iostream>
#include <cstring>

Lexer::Lexer(std::istream& input) : input(input) {

}

bool Lexer::isWhitespace(char c) {
    switch(c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
    }
    return false;
}

bool Lexer::isIdChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

char Lexer::read() {
    if(this->char_stack.size() > 0) {
        char result = this->char_stack.top();
        this->char_stack.pop();
        return result;
    }
    else {
        if(this->input.eof())
            return '\0';
        char c = this->input.get();
        if(c == '\n')
            ++this->linenr;
        if(this->input.eof())
            return '\0';
        return c;
    }
}

void Lexer::unread(char c) {
    this->char_stack.push(c);
}

Token Lexer::next_number() {
    bool is_float = false;
    std::stringstream ss;

    char c = this->read();
    while(c >= '0' && c <= '9') {
        ss << c;

        c = this->read();
    }
    if(c == '.') {
        is_float = true;
        ss << '.';
        c = this->read();
        while(c >= '0' && c <= '9') {
            ss << c;

            c = this->read();
        }
    }

    this->unread(c);

    std::string s = ss.str();
    try {
        if(is_float) {
            float data_f = std::stof(s);
            uint32_t data;
            std::memcpy(&data, &data_f, sizeof(float));
            return Token(TokenType::FLOAT, data);
        }
        else {
            unsigned data = std::stoul(s);
            return Token(TokenType::INTEGER, data);
        }
    }
    catch(const std::invalid_argument& e) {
        throw ParseException("Invalid number ", s);
    }
    catch(const std::out_of_range& e) {
        throw ParseException("Number out of range ", s);
    }
}

Token Lexer::next_id() {
    std::stringstream ss;

    char c = this->read();
    while(this->isIdChar(c) || (c >= '0' && c <= '9')) {
        ss << c;

        c = this->read();
    }

    this->unread(c);

    std::string s = ss.str();

    if(s == "if")
        return Token(TokenType::IF);
    else if(s == "else")
        return Token(TokenType::ELSE);
    else if(s == "while")
        return Token(TokenType::WHILE);
    else if(s == "int")
        return Token(TokenType::INT);
    else if(s == "float")
        return Token(TokenType::FLOAT);
    else if(s == "void")
        return Token(TokenType::VOID);
    else if(s == "function")
        return Token(TokenType::FUNCTION);
    else if(s == "return")
        return Token(TokenType::RETURN);
    else
        return Token(TokenType::ID, s);
}

Token Lexer::next_token() {
    char c = this->read();
    while(this->isWhitespace(c))
        c = this->read();

    switch(c) {
        case '+':
            return Token(TokenType::PLUS);
        case '-':
            return Token(TokenType::MIN);
        case '*':
            return Token(TokenType::MUL);
        case '/':
            return Token(TokenType::DIV);
        case '%':
            return Token(TokenType::MOD);
        case '^':
            return Token(TokenType::BITXOR);
        case '(':
            return Token(TokenType::OPEN_PAR);
        case ')':
            return Token(TokenType::CLOSE_PAR);
        case '{':
            return Token(TokenType::OPEN_CB);
        case '}':
            return Token(TokenType::CLOSE_CB);
        case ',':
            return Token(TokenType::COMMA);
        case ';':
            return Token(TokenType::SEMICOLON);
        case '@':
            return Token(TokenType::CAST);
        case '~':
            return Token(TokenType::BITNOT);
        case ':':
            return Token(TokenType::DECL);

        case '=': {
            c = this->read();
            if(c == '=')
                return Token(TokenType::EQ);
            else if(c == '>')
                return Token(TokenType::GREATEQ);
            else if(c == '<')
                return Token(TokenType::LESSEQ);
            this->unread(c);
            return Token(TokenType::ASSIGN);
        }
        case '>': {
            c = this->read();
            if(c == '=')
                return Token(TokenType::GREATEQ);
            else if(c == '>') {
                c = this->read();
                if(c == '>')
                    return Token(TokenType::URSHIFT);
                this->unread(c);
                return Token(TokenType::RSHIFT);
            }
            this->unread(c);
            return Token(TokenType::GREATER);
        }
        case '<': {
            c = this->read();
            if(c == '=')
                return TokenType(TokenType::LESSEQ);
            else if(c == '<')
                return TokenType(TokenType::LSHIFT);
            this->unread(c);
            return Token(TokenType::LESS);
        }
        case '!': {
            c = this->read();
            if(c == '=')
                return Token(TokenType::NEQ);
            this->unread(c);
            return Token(TokenType::NOT);
        }

        case '&': {
            c = this->read();
            if(c == '&')
                return TokenType(TokenType::LAND);
            this->unread(c);
            return TokenType(TokenType::BITAND);
        }
        case '|': {
            c = this->read();
            if(c == '|')
                return TokenType(TokenType::LOR);
            this->unread(c);
            return TokenType(TokenType::BITOR);
        }
    }

    if(c >= '0' && c <= '9') {
        this->unread(c);
        return this->next_number();
    }

    if(this->isIdChar(c)) {
        this->unread(c);
        return this->next_id();
    }

    if(c == '\0' && this->input.eof())
        return Token(TokenType::END_OF_FILE);

    throw ParseException("Unknown token ", c);
}

Token Lexer::lex() {
    if(this->token_stack.size() > 0) {
        Token result = this->token_stack.top();
        this->token_stack.pop();
        return result;
    }

    if(this->input.eof())
        return Token(TokenType::END_OF_FILE);

    return this->next_token();
}

void Lexer::unlex(const Token& token) {
    this->token_stack.push(token);
}

Token Lexer::lookahead() {
    Token result = this->lex();
    this->unlex(result);
    return result;
}