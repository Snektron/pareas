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
    auto seen = std::unordered_set<TransitionArray>();
    auto queue = std::deque<TransitionArray>();

    for (const auto& [sym, ta] : this->transition_table) {
        seen.insert({ta});
        queue.push_back(ta);
    }

    while (!queue.empty()) {
        auto first = queue.front();
        queue.pop_front();

        for (const auto& second : seen) {
            auto copy = first;
            copy.merge(second);

            if (seen.insert(copy).second)
                queue.push_back(copy);

            copy = second;
            copy.merge(first);

            if (seen.insert(copy).second)
                queue.push_back(copy);
        }
    }

    fmt::print("Parallel DFA has {} states\n", seen.size());
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
plus = /\+/
minus = /-/
slash = /\//
star = /\*/
semi = /;/
id = /[a-zA-Z_][a-zA-Z0-9_]*/
number = /[0-9]+/
whitespace = /[ \r\n][ \t\r\n]*/
comment = /\/\/[^\n]*\n/
)";

int main() {
    auto er = pareas::ErrorReporter(test_input, std::clog);
    auto parser = pareas::Parser(&er, test_input);
    auto lexer_parser = pareas::LexerParser(&parser);
    auto tokens = lexer_parser.parse();

    auto nfa = pareas::FiniteStateAutomaton::build_lexer_nfa({0, 127}, tokens);
    auto dfa = nfa.to_dfa();
    dfa.add_lexer_loop();

    fmt::print("Final DFA has {} states\n", dfa.num_states());
    // dfa.dump_dot(std::cout);

    auto transitions = std::vector<Transition>();
    for (size_t src = 0; src < dfa.num_states(); ++src) {
        for (const auto [sym, dst] : dfa[src].transitions) {
            assert(sym != pareas::FiniteStateAutomaton::EPSILON);
            transitions.push_back({.src = static_cast<int>(src), .dst = static_cast<int>(dst), .sym = static_cast<char>(sym)});
        }
    }

    auto pdfa = Dfa(transitions);
    pdfa.test();

    return EXIT_SUCCESS;
}
