# obz::endian

Small helpers for encoding and decoding integer fields in binary buffers using explicit byte order.

This library solves one narrow problem: turning fixed-size integer values into bytes, and bytes back into integer values, without relying on the machine's native endianness.

Binary formats should describe their byte layout directly. The implementation writes and reads one byte at a time, so the result is the same on little-endian and big-endian hosts.

---

## Features

- Little-endian read/write helpers
- Big-endian read/write helpers
- Works with `std::span<std::byte>`
- Supports signed and unsigned integer types except `bool`
- Throws on out-of-range buffer access
- Header-only implementation

---

## Usage

```cpp
#include <obz/endian.hpp>

#include <array>
#include <cstdint>

std::array<std::byte, 4> buffer{};

obz::endian::write_be<std::uint32_t>(buffer, 0, 42);

const auto value = obz::endian::read_be<std::uint32_t>(buffer, 0);
```

---

## API

### write_le

```cpp
template <typename T>
void write_le(std::span<std::byte> buffer, std::size_t offset, T value);
```

Writes an integral value in little-endian order.

Throws `std::out_of_range` if the value would exceed the buffer bounds.

---

### read_le

```cpp
template <typename T>
T read_le(std::span<const std::byte> buffer, std::size_t offset);
```

Reads an integral value in little-endian order.

Throws `std::out_of_range` if the value would exceed the buffer bounds.

---

### write_be

```cpp
template <typename T>
void write_be(std::span<std::byte> buffer, std::size_t offset, T value);
```

Writes an integral value in big-endian order.

Throws `std::out_of_range` if the value would exceed the buffer bounds.

---

### read_be

```cpp
template <typename T>
T read_be(std::span<const std::byte> buffer, std::size_t offset);
```

Reads an integral value in big-endian order.

Throws `std::out_of_range` if the value would exceed the buffer bounds.

---

## Behaviour Summary

| Operation | Valid Range | Invalid Range |
|-----------|-------------|---------------|
| read      | returns decoded value | throws `std::out_of_range` |
| write     | writes bytes | throws `std::out_of_range` |

Only integer types are supported. Passing a non-integral type, or `bool`, fails at compile time.

---

## Design Notes

The API takes an explicit `std::span<std::byte>` and offset so callers can use it with arrays, vectors, packet buffers, and file buffers without copying.

Bounds checks use exceptions rather than `assert`, so invalid ranges are still rejected in release builds.

Signed values are converted through the corresponding unsigned type before shifting and masking. This keeps the byte extraction logic explicit and avoids relying on signed shift behavior.

`std::byte` is used instead of `char` or `std::uint8_t` to make the API state its intent: the buffer is raw storage, not text and not an integer array.

---

## Implementation Notes

Each operation follows the same small pattern:

1. Check that the requested type is supported.
2. Check that `offset + sizeof(T)` fits in the span.
3. Convert the value to an unsigned representation.
4. Move bytes into or out of the buffer with shifts and masks.

This is deliberately more verbose than copying object memory. It avoids strict-aliasing concerns, avoids alignment assumptions, and makes the wire format visible in the code.

---

## When to Use

Use `endian` when:

- writing binary protocol fields
- reading binary protocol fields
- serializing integer values into byte buffers
- tests need deterministic byte layout

---

## When Not to Use

Avoid `endian` when:

- serializing complex object graphs
- text formats are more appropriate
- native in-memory representation is sufficient

---

## Potential Extensions

Possible extensions, if future use cases justify them:

- append-style helpers for callers building byte buffers incrementally
- cursor-based reader/writer wrappers for structured binary formats
- IEEE 754 floating-point helpers using explicit bit-casting semantics

---

## License

Part of the `obz` project.
