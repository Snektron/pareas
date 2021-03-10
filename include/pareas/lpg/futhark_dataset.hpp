#ifndef _PAREAS_LPG_FUTHARK_DATASET_HPP
#define _PAREAS_LPG_FUTHARK_DATASET_HPP

#include <vector>
#include <span>
#include <array>
#include <istream>
#include <ostream>
#include <utility>
#include <cstdint>
#include <cassert>

namespace pareas::futhark {
    constexpr const char FUTHARK_BINARY_FORMAT_VERSION = 2;

    using TypeString = std::array<uint8_t, 4>;

    template <typename T>
    struct FutharkTraits;

    template <typename T>
    concept FutharkRepresentable = requires {
        { FutharkTraits<T>::type_string } -> std::convertible_to<TypeString>;
    };

    using Index = std::span<const uint64_t>;

    template <FutharkRepresentable T>
    class Array {
        std::vector<uint64_t> dims;
        std::vector<T> items;

    public:
        Array(std::vector<uint64_t>&& shape, const T& value = T());

        static Array<T> read(std::istream& in);

        void write(std::ostream& out) const;

        T& at(Index index);
        const T& at(Index index) const;

        T* data();
        const T* data() const;

    private:
        size_t offset(Index index) const;
    };

    template <FutharkRepresentable T>
    Array<T>::Array(std::vector<uint64_t>&& shape, const T& value):
        dims(std::move(shape)) {
        assert(this->dims.size() > 0 && this->dims.size() <= 255);
        size_t total_size = 1;
        for (auto axis_dim : this->dims) {
            total_size *= axis_dim;
        }
        this->items.resize(total_size, value);
    }

    template <FutharkRepresentable T>
    void Array<T>::write(std::ostream& out) const {
        auto type_string = FutharkTraits<T>::type_string;
        uint8_t rank = this->dims.size();

        out.put('b');
        out.put(FUTHARK_BINARY_FORMAT_VERSION);
        out.write(reinterpret_cast<const char*>(&rank), sizeof rank);
        out.write(reinterpret_cast<const char*>(type_string.data()), type_string.size());
        out.write(reinterpret_cast<const char*>(this->dims.data()), rank * sizeof(uint64_t));
        out.write(reinterpret_cast<const char*>(this->items.data()), this->items.size() * sizeof(T));
    }

    template <FutharkRepresentable T>
    T& Array<T>::at(Index index) {
        return this->items[this->offset(index)];
    }

    template <FutharkRepresentable T>
    const T& Array<T>::at(Index index) const {
        return this->items[this->offset(index)];
    }

    template <FutharkRepresentable T>
    T* Array<T>::data() {
        return this->items.data();
    }

    template <FutharkRepresentable T>
    const T* Array<T>::data() const {
        return this->items.data();
    }

    template <FutharkRepresentable T>
    size_t Array<T>::offset(Index index) const {
        assert(index.size() == this->dims.size());
        size_t offset = 0;
        size_t stride = 1;

        for (size_t i = index.size(); i-- > 0;) {
            assert(index[i] < this->dims[i]);
            offset += index[i] * stride;
            stride *= this->dims[i];
        }

        return offset;
    }

    // Bool is explicitly not covered here because it requires special attention.

    template <>
    struct FutharkTraits<int8_t> {
        constexpr const static TypeString type_string = {' ', ' ', 'i', '8'};
    };

    template <>
    struct FutharkTraits<uint8_t> {
        constexpr const static TypeString type_string = {' ', ' ', 'u', '8'};
    };

    template <>
    struct FutharkTraits<int16_t> {
        constexpr const static TypeString type_string = {' ', 'i', '1', '6'};
    };

    template <>
    struct FutharkTraits<uint16_t> {
        constexpr const static TypeString type_string = {' ', 'u', '1', '6'};
    };

    template <>
    struct FutharkTraits<int32_t> {
        constexpr const static TypeString type_string = {' ', 'i', '3', '2'};
    };

    template <>
    struct FutharkTraits<uint32_t> {
        constexpr const static TypeString type_string = {' ', 'u', '3', '2'};
    };

    template <>
    struct FutharkTraits<int64_t> {
        constexpr const static TypeString type_string = {' ', 'i', '6', '4'};
    };

    template <>
    struct FutharkTraits<uint64_t> {
        constexpr const static TypeString type_string = {' ', 'u', '6', '4'};
    };
}

#endif
