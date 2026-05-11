#pragma once

#include <obz/transport/endpoint.hpp>
#include <obz/transport/native_handle.hpp>
#include <obz/transport/udp_socket.hpp>

#include <cstddef>
#include <span>
#include <string>
#include <system_error>
#include <vector>

namespace obz::transport::detail {

native_socket_handle invalid_socket() noexcept;
bool is_valid(native_socket_handle handle) noexcept;

void close_socket(native_socket_handle handle) noexcept;
std::system_error last_socket_error(const std::string& message);

native_socket_handle create_tcp_socket();
native_socket_handle create_udp_socket();

void connect_socket(native_socket_handle handle, const endpoint& remote_endpoint);
void bind_socket(native_socket_handle handle, const endpoint& local_endpoint);
void set_reuse_address(native_socket_handle handle);
void listen_socket(native_socket_handle handle, int backlog);
native_socket_handle accept_socket(native_socket_handle handle);

std::size_t send_tcp(native_socket_handle handle, std::span<const std::byte> data);
std::vector<std::byte> receive_tcp(native_socket_handle handle, std::size_t max_bytes);

std::size_t send_udp(
    native_socket_handle handle,
    const endpoint& remote_endpoint,
    std::span<const std::byte> data);
datagram receive_udp(native_socket_handle handle, std::size_t max_bytes);

endpoint local_endpoint_for(native_socket_handle handle);

} // namespace obz::transport::detail
