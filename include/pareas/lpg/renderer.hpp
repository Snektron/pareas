#ifndef _PAREAS_LPG_RENDERER_HPP
#define _PAREAS_LPG_RENDERER_HPP

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <fstream>
#include <cstdint>

namespace pareas {
    struct RenderError : std::runtime_error {
        RenderError(const std::string& msg): std::runtime_error(msg) {}
    };

    struct Renderer {
        const char* namesp;

        std::ofstream fut;
        std::ofstream hpp;
        std::ofstream cpp;
        std::ofstream dat;

        Renderer(const char* namesp, const std::filesystem::path& output);

        void finalize();

        void align_data(size_t align);
        size_t data_offset();

        void write_data_int(uint64_t value, size_t bytes);

        std::string render_offset_cast(size_t offset, std::string_view type) const;
    };
}

#endif
