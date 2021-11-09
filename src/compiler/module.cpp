#include "pareas/compiler/module.hpp"
#include "pareas/compiler/futhark_interop.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <utility>

void HostModule::dump(std::ostream& os) const {
    fmt::print("Total instructions: {}\n", this->num_instructions);
    for (size_t i = 0; i < this->num_functions; ++i) {
        fmt::print(os, "function {}:\n", i);
        fmt::print(os, "  id: {}\n", this->func_id[i]);
        fmt::print(os, "  start: {}\n", this->func_start[i]);
        fmt::print(os, "  size: {}\n", this->func_size[i]);
    }
}

DeviceModule::DeviceModule(futhark_context* ctx):
    ctx(ctx),
    func_id(nullptr),
    func_start(nullptr),
    func_size(nullptr),
    instructions(nullptr) {
}

DeviceModule::DeviceModule(DeviceModule&& other):
    ctx(std::exchange(other.ctx, nullptr)),
    func_id(std::exchange(other.func_id, nullptr)),
    func_start(std::exchange(other.func_start, nullptr)),
    func_size(std::exchange(other.func_size, nullptr)),
    instructions(std::exchange(other.instructions, nullptr)) {
}

DeviceModule& DeviceModule::operator=(DeviceModule&& other) {
    std::swap(this->ctx, other.ctx);
    std::swap(this->func_id, other.func_id);
    std::swap(this->func_start, other.func_start);
    std::swap(this->func_size, other.func_size);
    std::swap(this->instructions, other.instructions);
    return *this;
}

DeviceModule::~DeviceModule() {
    if (!this->ctx)
        return;

    if (this->func_id)
        futhark_free_u32_1d(this->ctx, this->func_id);

    if (this->func_start)
        futhark_free_u32_1d(this->ctx, this->func_start);

    if (this->func_size)
        futhark_free_u32_1d(this->ctx, this->func_size);

    if (this->instructions)
        futhark_free_u32_1d(this->ctx, this->instructions);
}

size_t DeviceModule::num_functions() const {
    if (!this->func_id)
        return 0;
    return futhark_shape_u32_1d(this->ctx, this->func_id)[0];
}


size_t DeviceModule::num_instructions() const {
    if (!this->instructions)
        return 0;
    return futhark_shape_u32_1d(this->ctx, this->instructions)[0];
}


HostModule DeviceModule::download() const {
    size_t num_functions = this->num_functions();
    size_t num_instructions = this->num_instructions();

    auto mod = HostModule{
        .num_functions = num_functions,
        .num_instructions = num_instructions,
        .func_id = std::make_unique<uint32_t[]>(num_functions),
        .func_start = std::make_unique<uint32_t[]>(num_functions),
        .func_size = std::make_unique<uint32_t[]>(num_functions),
        .instructions = std::make_unique<uint32_t[]>(num_instructions)
    };

    int err = futhark_values_u32_1d(this->ctx, this->func_id, mod.func_id.get());
    err |= futhark_values_u32_1d(this->ctx, this->func_start, mod.func_start.get());
    err |= futhark_values_u32_1d(this->ctx, this->func_size, mod.func_size.get());
    err |= futhark_values_u32_1d(this->ctx, this->instructions, mod.instructions.get());

    if (err)
        throw futhark::Error(this->ctx);

    return mod;
}
