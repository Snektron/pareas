#include <fmt/format.h>
#include <fmt/ostream.h>

#include "pareas/common/error_reporter.hpp"
#include "pareas/common/parser.hpp"
#include "pareas/common/hash_util.hpp"
#include "pareas/lexgen/lexer_parser.hpp"
#include "pareas/lexgen/fsa.hpp"

#include <iostream>
#include <vector>
#include <deque>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cstdlib>
#include <cassert>

constexpr const int REJECT_STATE = -1;

struct Transition {
    int src;
    int dst;
    char sym;
};

struct TransitionArray {
    std::vector<int> transitions;

    explicit TransitionArray(size_t states);

    void merge(const TransitionArray& other) {
        for (auto& state : this->transitions) {
            if (state != REJECT_STATE)
                state = other.transitions[state];
        }
    }

    void dump_csv() const {
        fmt::print("{}\n", fmt::join(this->transitions, ","));
    }
};

template<>
struct std::hash<TransitionArray> {
    size_t operator()(const TransitionArray& ta) const {
        size_t hash = 0;
        for (auto state : ta.transitions)
            hash = pareas::hash_combine(hash, state);
        return hash;
    }
};

bool operator==(const TransitionArray& lhs, const TransitionArray& rhs) {
    return std::equal(
        lhs.transitions.begin(),
        lhs.transitions.end(),
        rhs.transitions.begin(),
        rhs.transitions.end()
    );
}

TransitionArray::TransitionArray(size_t states):
    transitions(states, REJECT_STATE) {}

struct Dfa {
    std::unordered_map<char, TransitionArray> transition_table;
    size_t num_states;

    Dfa(std::span<const Transition> dfa);
    void dump_csv() const;
    void test() const;
};

Dfa::Dfa(std::span<const Transition> dfa) {
    auto seen_syms = std::unordered_set<char>();

    int max_state = -1;

    for (const auto [src, dst, _sym] : dfa) {
        assert(src != REJECT_STATE);
        max_state = std::max({max_state, src, dst});
    }

    assert(max_state >= 0);
    this->num_states = max_state + 1;

    for (const auto [src, dst, sym] : dfa) {
        auto it = this->transition_table.insert({sym, TransitionArray(this->num_states)}).first;
        it->second.transitions[src] = dst;
    }
}

void Dfa::dump_csv() const {
    for (size_t i = 0; i < this->num_states; ++i) {
        fmt::print(",{}", i);
    }
    fmt::print("\n");

    for (const auto& [sym, ta] : this->transition_table) {
        fmt::print("{},", sym);
        ta.dump_csv();
    }
}

void Dfa::test() const {
    auto seen = std::unordered_map<TransitionArray, std::string>();

    for (const auto& [sym, ta] : this->transition_table) {
        seen.insert({ta, std::string(1, sym)});
    }

    bool any_inserted = true;
    while (any_inserted) {
        any_inserted = false;

        auto new_tas = std::unordered_map<TransitionArray, std::string>();

        for (const auto& [ta1, syms1] : seen) {
            for (const auto& [ta2, syms2] : seen) {
                auto syms = syms1 + syms2;

                auto copy = ta1;
                copy.merge(ta2);

                new_tas[copy] = syms;
            }
        }

        for (const auto& elem : new_tas) {
            any_inserted |= seen.insert(elem).second;
        }
    }

    for (size_t i = 0; i < this->num_states; ++i) {
        fmt::print(",{}", i);
    }
    fmt::print("\n");
    for (const auto& [ta, syms] : seen) {
        fmt::print("{},", syms);
        ta.dump_csv();
    }
}

auto test_input = R"(
const = /const/
double = /double/
float = /float/
int = /int/
short = /short/
struct = /struct/
unsigned = /unsigned/
break = /break/
continue = /continue/
else = /else/
for = /for/
long = /long/
signed = /signed/
switch = /switch/
void = /void/
case = /case/
default = /default/
enum = /enum/
goto = /goto/
register = /register/
sizeof = /sizeof/
typedef = /typedef/
volatile = /volatile/
char = /char/
do = /do/
extern = /extern/
if = /if/
return = /return/
static = /static/
union = /union/
while = /while/
rparen = /\(/
lparen = /\)/
rbracket = /\[/
lbracket = /\]/
rbrace = /{/
lbrace = /}/
plus = /+/
minus = /-/
slash = /\//
star = /\*/
semi = /;/
id = /[a-zA-Z_][a-zA-Z0-9_]*/
number = /[0-9][0-9]*/
whitespace = /[ \r\n][ \t\r\n]*/
comment = /\/\/[^\n]*\n/
)";

int main() {
    auto er = pareas::ErrorReporter(test_input, std::clog);
    auto parser = pareas::Parser(&er, test_input);
    auto lexer_parser = pareas::LexerParser(&parser);
    auto tokens = lexer_parser.parse();

    auto dfa = pareas::FiniteStateAutomaton::build_lexer_dfa({0, 127}, tokens);
    fmt::print("Final DFA has {} states\n", dfa.num_states());

    // dfa.dump_dot(std::cout);

    // Transition transitions[] = {
    //     {.src = 0, .dst =  1, .sym = 'b'},
    //     {.src = 1, .dst =  2, .sym = 'a'},
    //     {.src = 2, .dst =  3, .sym = 'b'},
    // };

    // Transition transitions[] = {
    //     {.src = 0, .dst = 1, .sym = 'a'},
    //     {.src = 0, .dst = 2, .sym = 'b'},

    //     {.src = 1, .dst = 1, .sym = 'a'},
    //     {.src = 1, .dst = 3, .sym = 'b'},

    //     {.src = 2, .dst = 1, .sym = 'a'},
    //     {.src = 2, .dst = 2, .sym = 'b'},

    //     {.src = 3, .dst = 1, .sym = 'a'},
    //     {.src = 3, .dst = 4, .sym = 'b'},

    //     {.src = 4, .dst = 5, .sym = 'a'},
    //     {.src = 4, .dst = 6, .sym = 'b'},

    //     {.src = 5, .dst = 5, .sym = 'a'},
    //     {.src = 5, .dst = 7, .sym = 'b'},

    //     {.src = 6, .dst = 5, .sym = 'a'},
    //     {.src = 6, .dst = 6, .sym = 'b'},

    //     {.src = 7, .dst = 5, .sym = 'a'},
    //     {.src = 7, .dst = 8, .sym = 'b'},

    //     {.src = 8, .dst = 5, .sym = 'a'},
    //     {.src = 8, .dst = 6, .sym = 'b'},
    // };

    // auto dfa = Dfa(transitions);
    // dfa.dump_csv();
    // dfa.test();

    // auto fsa = pareas::FiniteStateAutomaton();
    // auto p = fsa.add_state(false, "p");
    // auto q = fsa.add_state(true, "q");

    // fsa.dump_dot(std::cout);

    return EXIT_SUCCESS;
}
