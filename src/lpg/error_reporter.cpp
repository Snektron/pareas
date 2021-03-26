#include "pareas/lpg/error_reporter.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <functional>

namespace pareas {
    ErrorReporter::ErrorReporter(std::string_view source, std::ostream& out):
        source(source), out(out), count(0) {

        for (size_t i = 0; i < this->source.size(); ++i) {
            if (this->source[i] == '\n')
                this->newlines.push_back(i);
        }
    }

    void ErrorReporter::error(SourceLocation loc, std::string_view msg) {
        this->print(loc, "error", msg);
    }

    void ErrorReporter::error(std::string_view msg) {
        this->print("error", msg);
    }

    void ErrorReporter::note(SourceLocation loc, std::string_view msg) {
        this->print(loc, "note", msg);
    }

    void ErrorReporter::note(std::string_view msg) {
        this->print("note", msg);
    }

    void ErrorReporter::print(SourceLocation loc, std::string_view tag, std::string_view msg) {
        auto info = this->line(loc);

        fmt::print(
            this->out,
            "{tag} at {line}:{column}: {msg}\n"
            "{src}\n"
            "{empty:>{column}}^\n",
            fmt::arg("tag", tag),
            fmt::arg("line", info.line),
            fmt::arg("column", info.column),
            fmt::arg("msg", msg),
            fmt::arg("src", info.text),
            fmt::arg("empty", "")
        );
    }

    void ErrorReporter::print(std::string_view tag, std::string_view msg) {
        fmt::print(
            this->out,
            "{tag}: {msg}\n",
            fmt::arg("tag", tag),
            fmt::arg("msg", msg)
        );
    }

    ErrorReporter::LineInfo ErrorReporter::line(SourceLocation loc) const {
        auto end_it = std::lower_bound(this->newlines.begin(), this->newlines.end(), loc.offset);
        auto i = static_cast<size_t>(std::distance(this->newlines.begin(), end_it));

        auto start = i == 0 ? 0 : this->newlines[i - 1] + 1;
        auto end = i == this->newlines.size() ? this->source.size() : this->newlines[i];

        return {
            .line = static_cast<size_t>(i) + 1, // one-indexed
            .column = loc.offset - start,
            .text = this->source.substr(start, end - start),
        };
    }
}
