#ifndef _PAREAS_JSON_FUTHARK_INTEROP_HPP
#define _PAREAS_JSON_FUTHARK_INTEROP_HPP

// This file is just copied for now.

#include "json_futhark_generated.h"

#include <memory>
#include <string>
#include <stdexcept>
#include <vector>
#include <utility>
#include <cstdint>
#include <cassert>
#include <cstdio>

namespace futhark {
    template <typename T, void(*deleter)(T*)>
    struct Deleter {
        void operator()(T* t) const {
            deleter(t);
        }
    };

    template <typename T, void(*deleter)(T*)>
    using Unique = std::unique_ptr<T, Deleter<T, deleter>>;

    using ContextConfig = Unique<futhark_context_config, futhark_context_config_free>;
    using Context = Unique<futhark_context, futhark_context_free>;

    inline std::string get_error_str(futhark_context* ctx) {
        auto err = futhark_context_get_error(ctx);
        if (err) {
            auto err_str = std::string(err);
            free(err); // leak if the string constructor throws, but whatever.
            return err_str;
        }

        return "(no diagnostic)";
    }

    struct Error: std::runtime_error {
        Error(Context& ctx):
            std::runtime_error(get_error_str(ctx.get())) {}

        Error(futhark_context* ctx):
            std::runtime_error(get_error_str(ctx)) {}
    };

    template <typename Array, int (*free_fn)(futhark_context* ctx, Array*)>
    struct UniqueOpaqueArray {
        futhark_context* ctx;
        Array* data;

        UniqueOpaqueArray(Context& ctx, Array* data):
            ctx(ctx.get()), data(data) {
        }

        explicit UniqueOpaqueArray(Context& ctx):
            ctx(ctx.get()), data(nullptr) {
        }

        UniqueOpaqueArray(UniqueOpaqueArray&& other):
            ctx(std::exchange(other.ctx, nullptr)), data(std::exchange(other.data, nullptr)) {
        }

        UniqueOpaqueArray& operator=(UniqueOpaqueArray&& other) {
            std::swap(this->data, other.data);
            std::swap(this->ctx, other.ctx);
        }

        UniqueOpaqueArray(const UniqueOpaqueArray&) = delete;
        UniqueOpaqueArray& operator=(const UniqueOpaqueArray&) = delete;

        ~UniqueOpaqueArray() {
            if (this->data) {
                free_fn(this->ctx, this->data);
            }
        }

        Array* get() {
            return this->data;
        }

        const Array* get() const {
            return this->data;
        }

        Array** operator&() {
            return &this->data;
        }
    };

    using UniqueLexTable = UniqueOpaqueArray<futhark_opaque_lex_table, futhark_free_opaque_lex_table>;
    using UniqueParseTable = UniqueOpaqueArray<futhark_opaque_parse_table, futhark_free_opaque_parse_table>;
    using UniqueStackChangeTable = UniqueOpaqueArray<futhark_opaque_stack_change_table, futhark_free_opaque_stack_change_table>;

    template <typename T, size_t N>
    struct ArrayTraits;

    template <typename T, size_t N>
    struct UniqueArray {
        using Array = typename ArrayTraits<T, N>::Array;

        UniqueOpaqueArray<Array, ArrayTraits<T, N>::free_fn> handle;

        UniqueArray(Context& ctx, Array* data):
            handle(ctx, data) {
        }

        explicit UniqueArray(Context& ctx):
            handle(ctx, nullptr) {
        }

        template <typename... Sizes>
        UniqueArray(Context& ctx, const T* data, Sizes... dims):
            handle(ctx, ArrayTraits<T, N>::new_fn(ctx.get(), data, dims...)) {
            if (!this->handle.data)
                throw Error(this->handle.ctx);
        }

        Array* get() {
            return this->handle.get();
        }

        const Array* get() const {
            return this->handle.get();
        }

        Array** operator&() {
            return &this->handle;
        }

        void values(T* out) const {
            int err = ArrayTraits<T, N>::values_fn(this->handle.ctx, this->handle.data, out);
            if (err != 0)
                throw Error(this->handle.ctx);
        }

        std::vector<T> download() const {
            auto* shape = this->shape();
            int64_t total_size = 1;
            for (size_t i = 0; i < N; ++i) {
                total_size *= shape[i];
            }

            auto result = std::vector<T>(total_size);
            this->values(result.data());
            return result;
        }

        const int64_t* shape() const {
            return ArrayTraits<T, N>::shape_fn(this->handle.ctx, this->handle.data);
        }
    };

    template <>
    struct ArrayTraits<uint8_t, 1> {
        using Array = futhark_u8_1d;
        constexpr static const auto new_fn = futhark_new_u8_1d;
        constexpr static const auto free_fn = futhark_free_u8_1d;
        constexpr static const auto shape_fn = futhark_shape_u8_1d;
        constexpr static const auto values_fn = futhark_values_u8_1d;
    };

    template <>
    struct ArrayTraits<uint16_t, 1> {
        using Array = futhark_u16_1d;
        constexpr static const auto new_fn = futhark_new_u16_1d;
        constexpr static const auto free_fn = futhark_free_u16_1d;
        constexpr static const auto shape_fn = futhark_shape_u16_1d;
        constexpr static const auto values_fn = futhark_values_u16_1d;
    };

    template <>
    struct ArrayTraits<uint16_t, 2> {
        using Array = futhark_u16_2d;
        constexpr static const auto new_fn = futhark_new_u16_2d;
        constexpr static const auto free_fn = futhark_free_u16_2d;
        constexpr static const auto shape_fn = futhark_shape_u16_2d;
        constexpr static const auto values_fn = futhark_values_u16_2d;
    };

    template <>
    struct ArrayTraits<int32_t, 1> {
        using Array = futhark_i32_1d;
        constexpr static const auto new_fn = futhark_new_i32_1d;
        constexpr static const auto free_fn = futhark_free_i32_1d;
        constexpr static const auto shape_fn = futhark_shape_i32_1d;
        constexpr static const auto values_fn = futhark_values_i32_1d;
    };

    template <>
    struct ArrayTraits<int32_t, 2> {
        using Array = futhark_i32_2d;
        constexpr static const auto new_fn = futhark_new_i32_2d;
        constexpr static const auto free_fn = futhark_free_i32_2d;
        constexpr static const auto shape_fn = futhark_shape_i32_2d;
        constexpr static const auto values_fn = futhark_values_i32_2d;
    };
}

#endif
