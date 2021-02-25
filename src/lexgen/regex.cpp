#include "pareas/lexgen/regex.hpp"
#include "pareas/common/escape.hpp"

#include <fmt/ostream.h>

namespace pareas {
    void SequenceNode::print(std::ostream& os) const {
        if (this->children.size() == 1) {
            this->children[0]->print(os);
        } else {
            fmt::print("(");
            for (const auto& child : children) {
                child->print(os);
            }
            fmt::print(")");
        }
    }

    void AlternationNode::print(std::ostream& os) const {
        if (this->children.size() == 1) {
            this->children[0]->print(os);
        } else {
            fmt::print("(");
            bool first = true;
            for (const auto& child : children) {
                if (first)
                    first = false;
                else
                    fmt::print("|");

                child->print(os);
            }
            fmt::print(")");
        }
    }

    void RepeatNode::print(std::ostream& os) const {
        this->child->print(os);
        fmt::print(os, "*");
    }

    void CharGroupNode::print(std::ostream& os) const {
        fmt::print(os, "[{}", this->invert ? "" : "^");

        for (const auto [min, max] : this->ranges) {
            if (min == max)
                fmt::print(os, "{}", min);
            else
                fmt::print(os, "{}-{}", min, max);
        }

        fmt::print(os, "]");
    }

    void CharNode::print(std::ostream& os) const {
        fmt::print(os, "{:r}", EscapeFormatter{this->c});
    }

    void EmptyNode::print(std::ostream& is) const {
    }
}
