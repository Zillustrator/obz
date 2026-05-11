#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace obz::endian {

template <typename T>
void require_supported_integer() {
    static_assert(
        std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>,
        "endian operations require an integral type other than bool");
}

inline void require_range(std::size_t buffer_size, std::size_t offset, std::size_t width) {
    if (offset > buffer_size || buffer_size - offset < width) {
        throw std::out_of_range("endian operation exceeds buffer bounds");
    }
}

template <typename T>
void write_le(std::span<std::byte> buffer, std::size_t offset, T value) {
    require_supported_integer<T>();
    require_range(buffer.size(), offset, sizeof(T));

    using unsigned_type = std::make_unsigned_t<T>;
    auto current = static_cast<unsigned_type>(value);

    for (std::size_t i = 0; i < sizeof(T); ++i) {
        buffer[offset + i] = static_cast<std::byte>(current & unsigned_type{0xff});
        current >>= 8;
    }
}

template <typename T>
T read_le(std::span<const std::byte> buffer, std::size_t offset) {
    require_supported_integer<T>();
    require_range(buffer.size(), offset, sizeof(T));

    using unsigned_type = std::make_unsigned_t<T>;
    unsigned_type result{};

    for (std::size_t i = 0; i < sizeof(T); ++i) {
        const auto byte = static_cast<unsigned_type>(
            std::to_integer<std::uint8_t>(buffer[offset + i]));
        result |= static_cast<unsigned_type>(byte << (8 * i));
    }

    return static_cast<T>(result);
}

template <typename T>
void write_be(std::span<std::byte> buffer, std::size_t offset, T value) {
    require_supported_integer<T>();
    require_range(buffer.size(), offset, sizeof(T));

    using unsigned_type = std::make_unsigned_t<T>;
    const auto current = static_cast<unsigned_type>(value);

    for (std::size_t i = 0; i < sizeof(T); ++i) {
        const auto shift = 8 * (sizeof(T) - 1 - i);
        buffer[offset + i] = static_cast<std::byte>(
            (current >> shift) & unsigned_type{0xff});
    }
}

template <typename T>
T read_be(std::span<const std::byte> buffer, std::size_t offset) {
    require_supported_integer<T>();
    require_range(buffer.size(), offset, sizeof(T));

    using unsigned_type = std::make_unsigned_t<T>;
    unsigned_type result{};

    for (std::size_t i = 0; i < sizeof(T); ++i) {
        const auto byte = static_cast<unsigned_type>(
            std::to_integer<std::uint8_t>(buffer[offset + i]));
        result = static_cast<unsigned_type>((result << 8) | byte);
    }

    return static_cast<T>(result);
}

} // namespace obz::endian
