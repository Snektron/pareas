#ifndef _PAREAS_COMPILER_MODULE_HPP
#define _PAREAS_COMPILER_MODULE_HPP

#include "futhark_generated.h"

#include <cstdint>

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
};

#endif
