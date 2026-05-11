#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include <obz/transport/endpoint.hpp>
#include <obz/transport/native_handle.hpp>

namespace obz::transport {

class tcp_socket {
public:
    tcp_socket() = default;
    explicit tcp_socket(native_socket_handle socket_handle);

    tcp_socket(const tcp_socket&) = delete;
    tcp_socket& operator=(const tcp_socket&) = delete;

    tcp_socket(tcp_socket&& other) noexcept;
    tcp_socket& operator=(tcp_socket&& other) noexcept;

    ~tcp_socket();

    void connect(const endpoint& remote_endpoint);

    std::size_t send(std::span<const std::byte> data);
    std::vector<std::byte> receive(std::size_t max_bytes = 4096);

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;

private:
    native_socket_handle socket_handle_{invalid_native_socket_handle};
};

} // namespace obz::transport
