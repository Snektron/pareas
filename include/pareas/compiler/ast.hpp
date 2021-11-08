#ifndef _PAREAS_COMPILER_AST_HPP
#define _PAREAS_COMPILER_AST_HPP

#include "futhark_generated.h"
#include "pareas_grammar.hpp"

#include <memory>
#include <iosfwd>

#include <cstdint>

// Keep in sync with src/compiler/datanode_types.fut
enum class DataType : uint8_t {
    INVALID = 0,
    VOID = 1,
    INT = 2,
    FLOAT = 3,
    INT_REF = 4,
    FLOAT_REF = 5,
};

const char* data_type_name(DataType dt);

struct HostAst {
    size_t num_nodes;
    size_t num_functions;

    std::unique_ptr<grammar::Production[]> node_types;
    std::unique_ptr<int32_t[]> parents;
    std::unique_ptr<uint32_t[]> node_data;
    std::unique_ptr<DataType[]> data_types;
    std::unique_ptr<int32_t[]> node_depths;
    std::unique_ptr<int32_t[]> child_indexes;

    std::unique_ptr<uint32_t[]> fn_tab;

    void dump_dot(std::ostream& os) const;
};

struct DeviceAst {
    futhark_context* ctx;

    futhark_u8_1d* node_types;
    futhark_i32_1d* parents;
    futhark_u32_1d* node_data;
    futhark_u8_1d* data_types;
    futhark_i32_1d* node_depths;
    futhark_i32_1d* child_indexes;

    futhark_u32_1d* fn_tab;

    explicit DeviceAst(futhark_context* ctx);

    DeviceAst(const DeviceAst&) = delete;
    DeviceAst& operator=(const DeviceAst&) = delete;

    DeviceAst(DeviceAst&& other);
    DeviceAst& operator=(DeviceAst&& other);

    ~DeviceAst();

    size_t num_nodes() const;
    size_t num_functions() const;

    HostAst download() const;
};

#endif
