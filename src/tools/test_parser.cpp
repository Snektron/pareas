#include <iostream>
#include <string>
#include <iterator>
#include <chrono>

struct Parser {
    std::string input;
    size_t offset;

    int current() const {
        return this->offset >= this->input.size() ? -1 : this->input[this->offset];
    }

    void advance() {
        ++this->offset;
    }

    bool eat(char c) {
        if (this->current() == c) {
            this->advance();
            return true;
        }

        return false;
    }
};

bool expr(Parser& p);

bool atom(Parser& p) {
    return p.eat('a') || (p.eat('[') && expr(p) && p.eat(']'));
}

bool sum(Parser& p) {
    if (p.eat('+'))
        return atom(p) && sum(p);

    return true;
}

bool expr(Parser& p) {
    return atom(p) && sum(p);
}

bool program(Parser& p) {
    return p.eat('(') && expr(p) && p.eat(')');
}

bool futhark_program(Parser& p) {
    if (!p.eat('b'))
        return false;
    p.offset += 1 + 1 + 4 + 8;
    return program(p);
}

int main() {
    auto p = Parser{
        .input = std::string(
            std::istreambuf_iterator<char>(std::cin),
            std::istreambuf_iterator<char>()
        ),
        .offset = 0
    };

    auto start = std::chrono::high_resolution_clock::now();
    bool matches = futhark_program(p);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << (matches ? "true" : "false") << " (matched in " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us)" << std::endl;
}
