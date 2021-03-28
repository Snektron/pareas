#include "pareas/lpg/renderer.hpp"

#include <fmt/ostream.h>

#include <algorithm>
#include <string>

namespace {
    using namespace pareas;

    std::ofstream open_output(const char* basename, const char* ext) {
        auto filename = std::string(basename);
        filename.append(ext);

        auto out = std::ofstream(filename, std::ios::binary);
        if (!out) {
            throw RenderError(fmt::format("Failed to open output file '{}'\n", filename));
        }
        return out;
    }
}

namespace pareas {
    Renderer::Renderer(const char* namesp, const char* output):
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
            "\n"
            "namespace {1} {{\n",
            namesp_upper,
            this->namesp
        );
    }

    void Renderer::finalize() {
        fmt::print(this->hpp, "}}\n#endif\n");
    }
}
