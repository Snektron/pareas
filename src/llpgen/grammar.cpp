#include "pareas/llpgen/grammar.hpp"
#include "pareas/llpgen/hash_util.hpp"

#include <ostream>
#include <algorithm>
#include <cassert>

InvalidGrammarError::InvalidGrammarError(const std::string& msg):
    std::runtime_error(msg) {}

MultipleStartRulesError::MultipleStartRulesError():
    InvalidGrammarError("Start rule appears in multiple productions") {};

InvalidStartRuleError::InvalidStartRuleError():
    InvalidGrammarError("Start rule is not in right form") {}

bool Terminal::operator==(const Terminal& other) const {
    return this->name == other.name;
}

Terminal Terminal::null() {
    return Terminal{""};
}

bool Terminal::is_null() const {
    return this->name.size() == 0;
}

bool NonTerminal::operator==(const NonTerminal& other) const {
    return this->name == other.name;
}

Symbol::Symbol(Terminal t): is_terminal(true), name(t.name) {}

Symbol::Symbol(NonTerminal nt): is_terminal(false), name(nt.name) {}

bool Symbol::is_null() const {
    return this->is_terminal && this->name.size() == 0;
}

bool Symbol::operator==(const Symbol& other) const {
    return this->is_terminal == other.is_terminal && this->name == other.name;
}

Terminal Symbol::as_terminal() const {
    assert(this->is_terminal);
    return Terminal{this->name};
}

NonTerminal Symbol::as_non_terminal() const {
    assert(!this->is_terminal);
    return NonTerminal{this->name};
}

Grammar::Grammar(NonTerminal start, Terminal left_delim, Terminal right_delim):
    start(start), left_delim(left_delim), right_delim(right_delim) {}

void Grammar::add_rule(const Production& prod) {
    this->productions.push_back(prod);
}

void Grammar::dump(std::ostream& os) const {
    os << "Start symbol: " << Symbol(this->start) << " " << std::endl;
    for (const auto& prod : this->productions) {
        os << prod << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const Terminal& t) {
    return t.is_null() ? os << "ε" : os << t.name;
}

std::ostream& operator<<(std::ostream& os, const NonTerminal& nt) {
    return os << nt.name;
}

std::ostream& operator<<(std::ostream& os, const Symbol& sym) {
    if (sym.is_null())
        return os << "ε";
    else if (sym.is_terminal)
        return os << '"' << sym.name << '"';
    else
        return os << sym.name;
}

std::ostream& operator<<(std::ostream& os, const Production& prod) {
    os << Symbol(prod.lhs) << "\t->";
    for (const auto& sym : prod.rhs) {
        os << " " << sym;
    }
    return os;
}

size_t std::hash<Terminal>::operator()(const Terminal& t) const {
    return std::hash<std::string>{}(t.name);
}

size_t std::hash<NonTerminal>::operator()(const NonTerminal& nt) const {
    return std::hash<std::string>{}(nt.name);
}

size_t std::hash<Symbol>::operator()(const Symbol& sym) const {
    return hash_combine(
        std::hash<bool>{}(sym.is_terminal),
        std::hash<std::string>{}(sym.name)
    );
}

namespace literals {
    Terminal operator ""_t(const char* name, size_t len) {
        return Terminal{std::string(name, len)};
    }

    NonTerminal operator ""_nt(const char* name, size_t len) {
        return NonTerminal{std::string(name, len)};
    }
}