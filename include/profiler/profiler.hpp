#ifndef _PAREAS_PROFILER_PROFILER_HPP
#define _PAREAS_PROFILER_PROFILER_HPP

#include <iosfwd>
#include <chrono>
#include <vector>
#include <functional>

namespace pareas {
    struct Profiler {
        using SyncCallback = std::function<void()>;

        using Clock = std::chrono::high_resolution_clock;

        struct HistoryEntry {
            unsigned level;
            const char* name;
            Clock::duration elapsed;
        };

        unsigned max_level;
        unsigned level;

        SyncCallback sync_callback;
        std::vector<Clock::time_point> starts;
        std::vector<HistoryEntry> history;

        Profiler(unsigned max_level);

        void set_sync_callback(SyncCallback sync_callback = null_callback);

        void begin();
        void end(const char* name);

        void dump(std::ostream& os);

        template <typename F>
        void measure(const char* name, F f) {
            this->begin();
            f();
            this->end(name);
        }

        static void null_callback() {}
    };
}

#endif

