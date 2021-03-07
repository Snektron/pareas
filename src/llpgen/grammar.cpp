#include "pareas/llpgen/grammar.hpp"
#include "pareas/common/hash_util.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <ostream>
#include <algorithm>
#include <cassert>

namespace pareas {
    bool Terminal::is_empty() const {
        return this->type == Type::EMPTY;
    }

    bool Terminal::operator==(const Terminal& other) const {
        return this->type == other.type && this->name == other.name;
    }

    // Just give these some name that makes them nice to print
    const Terminal Terminal::EMPTY = {Type::EMPTY, "ε"};
    const Terminal Terminal::START_OF_INPUT = {Type::START_OF_INPUT, "⊢"};
    const Terminal Terminal::END_OF_INPUT = {Type::END_OF_INPUT, "⊣"};

    bool NonTerminal::operator==(const NonTerminal& other) const {
        return this->name == other.name;
    }

    Symbol::Symbol(Terminal t): name(t.name) {
        switch (t.type) {
            case Terminal::Type::USER_DEFINED:
                this->type = Type::USER_DEFINED_TERMINAL;
                break;
            case Terminal::Type::EMPTY:
                this->type = Type::EMPTY_TERMINAL;
                break;
            case Terminal::Type::START_OF_INPUT:
                this->type = Type::START_OF_INPUT_TERMINAL;
                break;
            case Terminal::Type::END_OF_INPUT:
                this->type = Type::END_OF_INPUT_TERMINAL;
                break;
        }
    }

    Symbol::Symbol(NonTerminal nt): type(Type::NON_TERMINAL), name(nt.name) {}

    bool Symbol::operator==(const Symbol& other) const {
        return this->type == other.type && this->name == other.name;
    }

    bool Symbol::is_empty_terminal() const {
        return this->type == Type::EMPTY_TERMINAL;
    }

    bool Symbol::is_terminal() const {
        return this->type != Type::NON_TERMINAL;
    }

    Terminal Symbol::as_terminal() const {
        switch (this->type) {
            case Type::USER_DEFINED_TERMINAL:
                return Terminal{Terminal::Type::USER_DEFINED, this->name};
            case Type::EMPTY_TERMINAL:
                return Terminal{Terminal::Type::EMPTY, this->name};
            case Type::START_OF_INPUT_TERMINAL:
                return Terminal{Terminal::Type::START_OF_INPUT, this->name};
            case Type::END_OF_INPUT_TERMINAL:
                return Terminal{Terminal::Type::END_OF_INPUT, this->name};
            case Type::NON_TERMINAL:
                assert(false); // Not a terminal
        }
    }

    NonTerminal Symbol::as_non_terminal() const {
        assert(this->type == Type::NON_TERMINAL);
        return NonTerminal{this->name};
    }

    size_t Production::arity() const {
        size_t arity = 0;
        for (const auto& sym : this->rhs) {
            if (!sym.is_terminal())
                ++arity;
        }
        return arity;
    }

    void Grammar::dump(std::ostream& os) const {
        os << "Start symbol: " << this->start()->lhs << " " << std::endl;
        for (const auto& prod : this->productions) {
            os << prod << std::endl;
        }
    }

    void Grammar::validate(ErrorReporter& er) const {
        // Tag uniqueness is already checked by the grammar parser, so skip that here.
        bool error = !this->check_production_definitions(er);
        error |= !this->check_start_rule(er);

        if (error)
            throw InvalidGrammarError("Invalid grammar");
    }

    const Production* Grammar::start() const {
        return &this->productions[START_INDEX];
    }

    bool Grammar::check_production_definitions(ErrorReporter& er) const {
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
                if (sym.is_terminal() || exists(sym.as_non_terminal()))
                    continue;

                er.error(prod.loc, fmt::format("Missing rule definition for '{}'", sym.name));
                error = true;
            }
        }

        return !error;
    }


    bool Grammar::check_start_rule(ErrorReporter& er) const {
        if (this->productions.size() <= START_INDEX) {
            er.error("Missing start rule");
            return false;
        }

        auto* start = &this->productions[START_INDEX];
        bool error = false;

        for (const auto& prod : this->productions) {
            if (&prod == start)
                continue;

            if (prod.lhs == start->lhs) {
                er.error(prod.loc, "Duplicate start rule definition");
                er.note(start->loc, "First defined here");
                error = true;
            }
        }

        if (start->rhs.empty() || start->rhs.front() != Terminal::START_OF_INPUT || start->rhs.back() != Terminal::END_OF_INPUT) {
            er.error(start->loc, "Start rule not in correct form");
            er.note(start->loc, fmt::format(
                "Expected form {} -> '{}' ... '{}';",
                start->lhs,
                Terminal::START_OF_INPUT,
                Terminal::END_OF_INPUT
            ));
            error = true;
        }

        return !error;
    }

    std::ostream& operator<<(std::ostream& os, const Terminal& t) {
        return os << t.name;
    }

    std::ostream& operator<<(std::ostream& os, const NonTerminal& nt) {
        return os << nt.name;
    }

    std::ostream& operator<<(std::ostream& os, const Symbol& sym) {
        if (sym.is_terminal())
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
            return Terminal{Terminal::Type::USER_DEFINED, std::string(name, len)};
        }

        NonTerminal operator ""_nt(const char* name, size_t len) {
            return NonTerminal{std::string(name, len)};
        }
    }
}

size_t std::hash<pareas::Terminal>::operator()(const pareas::Terminal& t) const {
    return pareas::hash_combine(
        std::hash<pareas::Terminal::Type>{}(t.type),
        std::hash<std::string>{}(t.name)
    );
}

size_t std::hash<pareas::NonTerminal>::operator()(const pareas::NonTerminal& nt) const {
    return std::hash<std::string>{}(nt.name);
}

size_t std::hash<pareas::Symbol>::operator()(const pareas::Symbol& sym) const {
    return pareas::hash_combine(
        std::hash<pareas::Symbol::Type>{}(sym.type),
        std::hash<std::string>{}(sym.name)
    );
}
