#include "pareas/llpgen/error_reporter.hpp"

#include <algorithm>
#include <functional>

ErrorReporter::ErrorReporter(std::string_view source, std::ostream& out):
    source(source), out(out), count(0) {

    for (size_t i = 0; i < this->source.size(); ++i) {
        if (this->source[i] == '\n')
            this->newlines.push_back(i);
    }
}

void ErrorReporter::error(SourceLocation loc, std::string_view msg) const {
    this->print(loc, "error", msg);
}

void ErrorReporter::note(SourceLocation loc, std::string_view msg) const {
    this->print(loc, "note", msg);
}

void ErrorReporter::print(SourceLocation loc, std::string_view tag, std::string_view msg) const {
    auto info = this->line(loc);
    this->out << tag << " at " << info.line << ":" << info.column << ": " << msg << "\n";
    this->out << info.text << "\n";
    for (size_t i = 0; i < info.column; ++i) {
        this->out << ' ';
    }
    this->out << '^' << std::endl;
}

ErrorReporter::LineInfo ErrorReporter::line(SourceLocation loc) const {
    auto end_it = std::lower_bound(this->newlines.begin(), this->newlines.end(), loc.offset);
    auto i = std::distance(this->newlines.begin(), end_it);

    auto start = i == 0 ? 0 : this->newlines[i - 1] + 1;
    auto end = i == this->newlines.size() ? this->source.size() : this->newlines[i];

    return {
        .line = static_cast<size_t>(i) + 1, // one-indexed
        .column = loc.offset - start,
        .text = this->source.substr(start, end - start),
    };
}

