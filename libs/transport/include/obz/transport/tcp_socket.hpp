#pragma once

#include <cstddef>
#include <span>

#include <obz/transport/endpoint.hpp>
#include <obz/transport/native_handle.hpp>

namespace obz::transport {

enum class receive_status {
    completed,
    peer_closed,
};

struct receive_result {
    std::size_t bytes_received{0};
    receive_status status{receive_status::peer_closed};
};

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
    void send_all(std::span<const std::byte> data);
    [[nodiscard]] receive_result receive_some(std::span<std::byte> destination);
    [[nodiscard]] receive_result receive_exactly(std::span<std::byte> destination);

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;

private:
    native_socket_handle socket_handle_{invalid_native_socket_handle};
};

} // namespace obz::transport
