#ifndef _PAREAS_COMPILER_PROFILER_HPP
#define _PAREAS_COMPILER_PROFILER_HPP

#include "futhark_generated.h"

#include <iosfwd>
#include <chrono>
#include <vector>

struct Profiler {
    using Clock = std::chrono::high_resolution_clock;

    struct HistoryEntry {
        unsigned level;
        const char* name;
        Clock::duration elapsed;
    };

    unsigned max_level;
    unsigned level;

    std::vector<Clock::time_point> starts;
    std::vector<HistoryEntry> history;

    Profiler(unsigned max_level);

    void begin(futhark_context* ctx = nullptr);
    void end(const char* name, futhark_context* ctx = nullptr);

    void dump(std::ostream& os);

    template <typename F>
    void measure(const char* name, F f) {
        this->begin();
        f();
        this->end(name);
    }

    template <typename F>
    void measure(const char* name, futhark_context* ctx, F f) {
        this->begin(ctx);
        f();
        this->end(name, ctx);
    }
};

#endif
