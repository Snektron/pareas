#ifndef _PAREAS_LPG_PARSER_LLP_TEST_PARSER_HPP
#define _PAREAS_LPG_PARSER_LLP_TEST_PARSER_HPP

#include "pareas/lpg/parser/grammar.hpp"
#include "pareas/lpg/parser/llp/parsing_table.hpp"

#include <vector>
#include <span>
#include <iosfwd>

namespace pareas::parser::llp {
    enum class BracketSide {
        LEFT,
        RIGHT,
    };

    struct Bracket {
        BracketSide side;
        Symbol sym;
    };

    class TestParser {
        const ParsingTable* llp_table;
        std::span<const Terminal> input;
        std::vector<Bracket> brackets;
        std::vector<const Production*> derivation;

    public:
        TestParser(const ParsingTable* llp_table, std::span<const Terminal> input);
        bool parse();
        void dump(std::ostream& out) const;

    private:
        bool compute_brackets();
        bool verify_brackets() const;
    };
}

#endif
