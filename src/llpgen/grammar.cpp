#include "pareas/llpgen/grammar.hpp"
#include "pareas/common/hash_util.hpp"

#include <fmt/format.h>

#include <ostream>
#include <algorithm>
#include <cassert>

namespace pareas {
    bool Terminal::operator==(const Terminal& other) const {
        return this->name == other.name;
    }

    Terminal Terminal::null() {
        return Terminal{""};
    }

    bool Terminal::is_null() const {
        return this->name.empty();
    }

    bool NonTerminal::operator==(const NonTerminal& other) const {
        return this->name == other.name;
    }

    Symbol::Symbol(Terminal t): is_terminal(true), name(t.name) {}

    Symbol::Symbol(NonTerminal nt): is_terminal(false), name(nt.name) {}

    bool Symbol::is_null() const {
        return this->is_terminal && this->name.empty();
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

    void Grammar::dump(std::ostream& os) const {
        os << "Start symbol: " << this->start->lhs << " " << std::endl;
        for (const auto& prod : this->productions) {
            os << prod << std::endl;
        }
    }

    void Grammar::validate(ErrorReporter& er) const {
        // Tags are already guaranteed to be unique by the parser, so we just need to check
        // whether rules exist here.
        bool error = false;

        auto exists = [&](const auto& lhs) {
            for (const auto& prod : this->productions) {
                if (prod.lhs == lhs)
                    return true;
            }

            return false;
        };

        for (const auto& prod : this->productions) {
            for (const auto& sym : prod.rhs) {
                if (sym.is_terminal || exists(sym.as_non_terminal()))
                    continue;

                er.error(prod.loc, fmt::format("Missing rule definition for '{}'", sym.name));
                error = true;
            }
        }

        if (error)
            throw MissingRuleDefinitionError();
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
            return os << '\'' << sym.name << '\'';
        else
            return os << sym.name;
    }

    std::ostream& operator<<(std::ostream& os, const Production& prod) {
        os << Symbol(prod.lhs) << " [" << prod.tag << "] ->";
        if (prod.rhs.empty()) {
            os << " ε";
        } else {
            for (const auto& sym : prod.rhs)
                os << " " << sym;
        }

        return os;
    }

    namespace literals {
        Terminal operator ""_t(const char* name, size_t len) {
            return Terminal{std::string(name, len)};
        }

        NonTerminal operator ""_nt(const char* name, size_t len) {
            return NonTerminal{std::string(name, len)};
        }
    }
}

size_t std::hash<pareas::Terminal>::operator()(const pareas::Terminal& t) const {
    return std::hash<std::string>{}(t.name);
}

size_t std::hash<pareas::NonTerminal>::operator()(const pareas::NonTerminal& nt) const {
    return std::hash<std::string>{}(nt.name);
}

size_t std::hash<pareas::Symbol>::operator()(const pareas::Symbol& sym) const {
    return pareas::hash_combine(
        std::hash<bool>{}(sym.is_terminal),
        std::hash<std::string>{}(sym.name)
    );
}
