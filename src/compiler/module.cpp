#include "pareas/compiler/module.hpp"

#include <utility>

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
