#include "pareas/compiler/profiler.hpp"
#include "pareas/compiler/futhark_interop.hpp"

#include <fmt/ostream.h>
#include <fmt/chrono.h>

#include <cstddef>
#include <cassert>

Profiler::Profiler(unsigned max_level):
    max_level(max_level),
    level(0) {
}

void Profiler::begin(futhark_context* ctx) {
    ++this->level;
    if (this->level > this->max_level)
        return;

    if (ctx && futhark_context_sync(ctx))
        throw futhark::Error(ctx);

    auto start = Clock::now();
    this->starts.push_back(start);
}

void Profiler::end(const char* name, futhark_context* ctx) {
    --this->level;
    if (this->level >= this->max_level)
        return;

    if (ctx && futhark_context_sync(ctx))
        throw futhark::Error(ctx);

    auto end = Clock::now();
    auto start = this->starts.back();
    this->starts.pop_back();
    auto diff = end - start;
    this->history.push_back(HistoryEntry{this->level, name, diff});
}

void Profiler::dump(std::ostream& os) {
    assert(this->level == 0);

    auto ordered_history = std::vector<HistoryEntry>(this->history.size());
    auto level_index_stack = std::vector<size_t>();

    size_t j = 0;
    for (const auto& entry : this->history) {
        for (unsigned i = level_index_stack.size(); i <= entry.level; ++i) {
            level_index_stack.push_back(j++);
        }

        size_t index = level_index_stack.back();
        ordered_history[index] = entry;
        level_index_stack.pop_back();
    }

    auto name_stack = std::vector<const char*>();
    for (auto [level, name, elapsed] : ordered_history) {
        name_stack.resize(level);
        name_stack.push_back(name);

        auto us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
        fmt::print("{}: {}\n", fmt::join(name_stack, "."), us);
    }
}
