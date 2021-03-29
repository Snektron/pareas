#include "pareas/lpg/renderer.hpp"

#include <fmt/ostream.h>

#include <algorithm>
#include <string>
#include <bit>
#include <cassert>

namespace {
    using namespace pareas;

    std::ofstream open_output(const std::filesystem::path& output, const char* ext) {
        auto filename = output;
        filename.replace_extension(ext);
        auto out = std::ofstream(filename, std::ios::binary);
        if (!out) {
            throw RenderError(fmt::format("Failed to open output file '{}'\n", filename));
        }
        return out;
    }
}

namespace pareas {
    Renderer::Renderer(const char* namesp, const std::filesystem::path& output):
        namesp(namesp),
        fut(open_output(output, ".fut")),
        hpp(open_output(output, ".hpp")),
        cpp(open_output(output, ".cpp")),
        dat(open_output(output, ".dat")) {

        auto namesp_upper = std::string(this->namesp);
        std::transform(namesp_upper.begin(), namesp_upper.end(), namesp_upper.begin(), ::toupper);

        fmt::print(
            this->hpp,
            "#ifndef _{0}_HPP\n"
            "#define _{0}_HPP\n"
            "\n"
            "#include <cstdint>\n"
            "#include <cstddef>\n"
            "\n"
            "namespace {1} {{\n",
            namesp_upper,
            this->namesp
        );

        fmt::print(
            this->cpp,
            "#include \"{0}.hpp\"\n"
            "extern \"C\" const uint8_t _{1}_data[];\n"
            "namespace {1} {{\n",
            output.filename().c_str(),
            this->namesp
        );

        auto asm_out = open_output(output, ".S");
        fmt::print(
            asm_out,
            "    .global _{0}_data\n"
            "    .align 8\n"
            "_{0}_data:\n"
            "    .incbin \"{1}.dat\"\n",
            namesp,
            output.filename().c_str()
        );
    }

    void Renderer::finalize() {
        fmt::print(this->hpp, "}}\n#endif\n");
        fmt::print(this->cpp, "}}\n");
    }

    void Renderer::align_data(size_t align) {
        auto offset = this->data_offset();
        auto diff = (align - offset % align) % align;

        for (size_t i = 0; i < diff; ++i) {
            this->dat.put(0);
        }
    }

    size_t Renderer::data_offset() {
        auto p = this->dat.tellp();
        assert(p >= 0);
        return static_cast<size_t>(p);
    }

    void Renderer::write_data_int(uint64_t value, size_t bytes) {
        // If the system is little endian, a value can be truncated simply by writing less bytes.
        static_assert(std::endian::native == std::endian::little);
        assert(value < (1ULL << (8ULL * bytes)));

        this->dat.write(reinterpret_cast<char*>(&value), bytes);
    }

    std::string Renderer::render_offset_cast(size_t offset, std::string_view type) const {
        return fmt::format("reinterpret_cast<const {}*>(_{}_data + {})", type, this->namesp, offset);
    }
}
