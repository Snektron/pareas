#ifndef _PAREAS_LPG_FUTHARK_DATASET_HPP
#define _PAREAS_LPG_FUTHARK_DATASET_HPP

#include <span>
#include <iosfwd>
#include <concepts>
#include <array>
#include <cstdint>
#include <cassert>

namespace pareas {
    constexpr const char FUTHARK_BINARY_FORMAT_VERSION = 2;

    using FutharkTypeString = std::array<uint8_t, 4>;

    template <typename T>
    struct FutharkTraits;

    template <typename T>
    concept FutharkRepresentable = requires {
        { FutharkTraits<T>::type_string } -> std::convertible_to<FutharkTypeString>;
    };

    template <FutharkRepresentable T>
    void write_futhark_array(std::ostream& out, std::span<uint64_t> dims, std::span<const T> data) {
        // This function uses a few questionable constructs, like reinterpret casting a
        // uint64_t (which requires the host architecture to be little endian as guarded in
        // meson.build), and reinterpreting characters to chars. Accepting a proper
        // basic_ostream<uint8_t> is too much effort, so just hack it with reinterpret casts.

        auto total_size = 1;
        for (auto axis_dim : dims) {
            total_size *= axis_dim;
        }
        assert(total_size == data.size());
        assert(dims.size() <= 255);

        auto type_string = FutharkTraits<T>::type_string;
        out.put('b');
        out.put(FUTHARK_BINARY_FORMAT_VERSION);
        uint8_t dim = dims.size();
        out.write(reinterpret_cast<const char*>(&dim), sizeof dim);
        out.write(reinterpret_cast<const char*>(type_string.data()), type_string.size());

        for (auto axis_dim : dims) {
            out.write(reinterpret_cast<const char*>(&axis_dim), sizeof axis_dim);
        }

        out.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
    }

    template <FutharkRepresentable T>
    void write_futhark_scalar(std::ostream& out, const T& value) {
        write_futhark_array(out, {}, std::span<const T>(&value, 1));
    }

    template <>
    struct FutharkTraits<int8_t> {
        constexpr const static FutharkTypeString type_string = {' ', ' ', 'i', '8'};
    };

    template <>
    struct FutharkTraits<uint8_t> {
        constexpr const static FutharkTypeString type_string = {' ', ' ', 'u', '8'};
    };

    template <>
    struct FutharkTraits<int16_t> {
        constexpr const static FutharkTypeString type_string = {' ', 'i', '1', '6'};
    };

    template <>
    struct FutharkTraits<uint16_t> {
        constexpr const static FutharkTypeString type_string = {' ', 'u', '1', '6'};
    };

    template <>
    struct FutharkTraits<int32_t> {
        constexpr const static FutharkTypeString type_string = {' ', 'i', '3', '2'};
    };

    template <>
    struct FutharkTraits<uint32_t> {
        constexpr const static FutharkTypeString type_string = {' ', 'u', '3', '2'};
    };

    template <>
    struct FutharkTraits<int64_t> {
        constexpr const static FutharkTypeString type_string = {' ', 'i', '6', '4'};
    };

    template <>
    struct FutharkTraits<uint64_t> {
        constexpr const static FutharkTypeString type_string = {' ', 'u', '6', '4'};
    };

    template <>
    struct FutharkTraits<bool> {
        constexpr const static FutharkTypeString type_string = {'b', 'o', 'o', 'l'};
    };
}

#endif
