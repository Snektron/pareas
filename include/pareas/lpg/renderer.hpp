#ifndef _PAREAS_LPG_RENDERER_HPP
#define _PAREAS_LPG_RENDERER_HPP

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <fstream>

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

        void align_dat(size_t align);
        size_t dat_offset();
        std::string render_offset_cast(size_t offset, std::string_view type) const;
    };
}

#endif
