#include "profiler/profiler.hpp"

#include <fmt/ostream.h>
#include <fmt/chrono.h>

#include <cstddef>
#include <cassert>

namespace pareas {
    Profiler::Profiler(unsigned max_level):
        max_level(max_level),
        level(0),
        sync_callback(null_callback) {
    }

    void Profiler::set_sync_callback(SyncCallback sync_callback) {
        this->sync_callback = sync_callback;
    }

    void Profiler::begin() {
        ++this->level;
        if (this->level > this->max_level)
            return;

        this->sync_callback();

        auto start = Clock::now();
        this->starts.push_back(start);
    }

    void Profiler::end(const char* name) {
        --this->level;
        if (this->level >= this->max_level)
            return;

        this->sync_callback();

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
}

