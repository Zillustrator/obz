#pragma once

#include <cstddef>
#include <new>

namespace obz::detail {

// This value affects object layout: use consistent settings across translation units.
#if defined(__cpp_lib_hardware_interference_size) && __cpp_lib_hardware_interference_size >= 201703L
inline constexpr std::size_t destructive_interference_size{std::hardware_destructive_interference_size};
#else // A fallback separation policy, not a measurement of the hardware's cache-line size.
inline constexpr std::size_t destructive_interference_size{64};
#endif

} // namespace obz::detail
