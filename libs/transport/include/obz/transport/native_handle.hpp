#pragma once

#include <cstdint>

namespace obz::transport {

#ifdef _WIN32
using native_socket_handle = std::uintptr_t;
#else
using native_socket_handle = int;
#endif

inline constexpr native_socket_handle invalid_native_socket_handle =
    static_cast<native_socket_handle>(-1);

} // namespace obz::transport
