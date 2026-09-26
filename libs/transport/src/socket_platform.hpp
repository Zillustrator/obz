#pragma once

#include <obz/transport/endpoint.hpp>
#include <obz/transport/native_handle.hpp>
#include <obz/transport/udp_socket.hpp>

#include <cstddef>
#include <span>
#include <string>

namespace obz::transport::detail {

native_socket_handle invalid_socket() noexcept;
bool is_valid(native_socket_handle handle) noexcept;

void close_socket(native_socket_handle handle) noexcept;

native_socket_handle create_tcp_socket();
native_socket_handle create_udp_socket();

void connect_socket(native_socket_handle handle, const endpoint& remote_endpoint);
void bind_socket(native_socket_handle handle, const endpoint& local_endpoint);
void set_reuse_address(native_socket_handle handle);
void listen_socket(native_socket_handle handle, int backlog);
native_socket_handle accept_socket(native_socket_handle handle);

std::size_t send_tcp(native_socket_handle handle, std::span<const std::byte> data);
std::size_t receive_tcp(native_socket_handle handle, std::span<std::byte> destination);

std::size_t send_udp(
    native_socket_handle handle,
    const endpoint& remote_endpoint,
    std::span<const std::byte> data);
udp_receive_result receive_udp(
    native_socket_handle handle,
    std::span<std::byte> destination);

void join_multicast_group(
    native_socket_handle handle,
    const std::string& group_address,
    const std::string& interface_address);
void leave_multicast_group(
    native_socket_handle handle,
    const std::string& group_address,
    const std::string& interface_address);

endpoint local_endpoint_for(native_socket_handle handle);

} // namespace obz::transport::detail
