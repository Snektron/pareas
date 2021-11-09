#ifndef _PAREAS_COMPILER_MODULE_HPP
#define _PAREAS_COMPILER_MODULE_HPP

#include "futhark_generated.h"

#include <memory>
#include <iosfwd>
#include <cstdint>

struct HostModule {
    size_t num_functions;
    size_t num_instructions;

    std::unique_ptr<uint32_t[]> func_id;
    std::unique_ptr<uint32_t[]> func_start;
    std::unique_ptr<uint32_t[]> func_size;

    std::unique_ptr<uint32_t[]> instructions;

    void dump(std::ostream& os) const;
};

struct DeviceModule {
    futhark_context* ctx;

    futhark_u32_1d* func_id;
    futhark_u32_1d* func_start;
    futhark_u32_1d* func_size;
    futhark_u32_1d* instructions;

    explicit DeviceModule(futhark_context* ctx);

    DeviceModule(const DeviceModule&) = delete;
    DeviceModule& operator=(const DeviceModule&) = delete;

    DeviceModule(DeviceModule&& other);
    DeviceModule& operator=(DeviceModule&& other);

    ~DeviceModule();

    size_t num_functions() const;
    size_t num_instructions() const;

    HostModule download() const;
};

#endif
