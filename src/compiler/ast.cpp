#include "pareas/compiler/ast.hpp"
#include "pareas/compiler/futhark_interop.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <utility>

const char* data_type_name(DataType dt) {
    switch (dt) {
        case DataType::INVALID: return "invalid";
        case DataType::VOID: return "void";
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::INT_REF: return "int ref";
        case DataType::FLOAT_REF: return "float ref";
    }
}

void HostAst::dump_dot(std::ostream& os) const {
    fmt::print(os, "digraph prog {{\n");

    for (size_t i = 0; i < this->num_nodes; ++i) {
        auto prod = this->node_types[i];
        auto parent = this->parents[i];
        auto* name = grammar::production_name(prod);

        if (parent != i) {
            fmt::print(
                os,
                "node{} [label=\"{}\nindex={}\ndepth={}\nchild index={}",
                i,
                name,
                i,
                this->node_depths[i],
                this->child_indexes[i]
            );

            switch (prod) {
                case grammar::Production::ATOM_NAME:
                case grammar::Production::ATOM_DECL:
                case grammar::Production::ATOM_DECL_EXPLICIT:
                    fmt::print(os, "\\n(offset={})", this->node_data[i]);
                    break;
                case grammar::Production::FN_DECL:
                    fmt::print(os, "\\n(num locals={})", this->fn_tab[this->node_data[i]]);
                case grammar::Production::ATOM_FN_CALL:
                    fmt::print(os, "\\n(fn id={})", this->node_data[i]);
                    break;
                case grammar::Production::PARAM:
                case grammar::Production::ARG:
                    fmt::print(os, "\\n(arg id={})", this->node_data[i]);
                    break;
                case grammar::Production::ATOM_INT:
                    fmt::print(os, "\\n(value={})", this->node_data[i]);
                    break;
                case grammar::Production::ATOM_FLOAT:
                    fmt::print(os, "\\n(value={})", *reinterpret_cast<const float*>(&this->node_data[i]));
                    break;
                default:
                    if (this->node_data[i] != 0) {
                        fmt::print(os, "\\n(junk={})", this->node_data[i]);
                    }
            }

            fmt::print(os, "\\n[{}]", data_type_name(this->data_types[i]));
            fmt::print(os, "\"]\n");

            if (parent >= 0) {
                fmt::print(os, "node{} -> node{};\n", parent, i);
            } else {
                fmt::print(os, "start{0} [style=invis];\nstart{0} -> node{0};\n", i);
            }
        }
    }

    fmt::print(os, "}}\n");
}


DeviceAst::DeviceAst(futhark_context* ctx):
    ctx(ctx),
    node_types(nullptr),
    parents(nullptr),
    node_data(nullptr),
    data_types(nullptr),
    node_depths(nullptr),
    child_indexes(nullptr),
    fn_tab(nullptr) {
}

DeviceAst::DeviceAst(DeviceAst&& other):
    ctx(std::exchange(other.ctx, nullptr)),
    node_types(std::exchange(other.node_types, nullptr)),
    parents(std::exchange(other.parents, nullptr)),
    node_data(std::exchange(other.node_data, nullptr)),
    data_types(std::exchange(other.data_types, nullptr)),
    node_depths(std::exchange(other.node_depths, nullptr)),
    child_indexes(std::exchange(other.child_indexes, nullptr)),
    fn_tab(std::exchange(other.fn_tab, nullptr)) {
}

DeviceAst& DeviceAst::operator=(DeviceAst&& other) {
    std::swap(this->ctx, other.ctx);
    std::swap(this->node_types, other.node_types);
    std::swap(this->parents, other.parents);
    std::swap(this->node_data, other.node_data);
    std::swap(this->data_types, other.data_types);
    std::swap(this->node_depths, other.node_depths);
    std::swap(this->child_indexes, other.child_indexes);
    std::swap(this->fn_tab, other.fn_tab);
    return *this;
}

DeviceAst::~DeviceAst() {
    if (!this->ctx)
        return;

    if (this->node_types)
        futhark_free_u8_1d(this->ctx, this->node_types);

    if (this->parents)
        futhark_free_i32_1d(this->ctx, this->parents);

    if (this->node_data)
        futhark_free_u32_1d(this->ctx, this->node_data);

    if (this->data_types)
        futhark_free_u8_1d(this->ctx, this->data_types);

    if (this->node_depths)
        futhark_free_i32_1d(this->ctx, this->node_depths);

    if (this->child_indexes)
        futhark_free_i32_1d(this->ctx, this->child_indexes);

    if (this->fn_tab)
        futhark_free_u32_1d(this->ctx, this->fn_tab);
}

size_t DeviceAst::num_nodes() const {
    if (!this->node_types)
        return 0;
    return futhark_shape_u8_1d(this->ctx, this->node_types)[0];
}

size_t DeviceAst::num_functions() const {
    if (!this->fn_tab)
        return 0;
    return futhark_shape_u32_1d(this->ctx, this->fn_tab)[0];
}

HostAst DeviceAst::download() const {
    size_t num_nodes = this->num_nodes();
    size_t num_functions = this->num_functions();

    auto ast = HostAst{
        .num_nodes = num_nodes,
        .num_functions = num_functions,
        .node_types = std::make_unique<grammar::Production[]>(num_nodes),
        .parents = std::make_unique<int32_t[]>(num_nodes),
        .node_data = std::make_unique<uint32_t[]>(num_nodes),
        .data_types = std::make_unique<DataType[]>(num_nodes),
        .node_depths = std::make_unique<int32_t[]>(num_nodes),
        .child_indexes = std::make_unique<int32_t[]>(num_nodes),
        .fn_tab = std::make_unique<uint32_t[]>(num_functions)
    };

    int err = futhark_values_u8_1d(
        this->ctx,
        this->node_types,
        reinterpret_cast<std::underlying_type_t<grammar::Production>*>(ast.node_types.get())
    );

    err |= futhark_values_i32_1d(this->ctx, this->parents, ast.parents.get());
    err |= futhark_values_u32_1d(this->ctx, this->node_data, ast.node_data.get());

    err |= futhark_values_u8_1d(
        this->ctx,
        this->data_types,
        reinterpret_cast<std::underlying_type_t<DataType>*>(ast.data_types.get())
    );

    err |= futhark_values_i32_1d(this->ctx, this->node_depths, ast.node_depths.get());
    err |= futhark_values_i32_1d(this->ctx, this->child_indexes, ast.child_indexes.get());

    err |= futhark_values_u32_1d(this->ctx, this->fn_tab, ast.fn_tab.get());

    if (err)
        throw futhark::Error(this->ctx);

    return ast;
}