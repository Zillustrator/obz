#pragma once

#include <cstdint>

#include <obz/transport/endpoint.hpp>
#include <obz/transport/native_handle.hpp>
#include <obz/transport/tcp_socket.hpp>

namespace obz::transport {

class tcp_listener {
public:
    tcp_listener() = default;

    tcp_listener(const tcp_listener&) = delete;
    tcp_listener& operator=(const tcp_listener&) = delete;

    tcp_listener(tcp_listener&& other) noexcept;
    tcp_listener& operator=(tcp_listener&& other) noexcept;

    ~tcp_listener();

    void listen(const endpoint& local_endpoint, int backlog = 8);
    tcp_socket accept();

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;

private:
    native_socket_handle listen_handle_{invalid_native_socket_handle};
};

} // namespace obz::transport
