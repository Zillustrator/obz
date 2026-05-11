#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include <obz/transport/endpoint.hpp>
#include <obz/transport/native_handle.hpp>

namespace obz::transport {

struct datagram {
    endpoint sender;
    std::vector<std::byte> payload;
};

class udp_socket {
public:
    udp_socket() = default;

    udp_socket(const udp_socket&) = delete;
    udp_socket& operator=(const udp_socket&) = delete;

    udp_socket(udp_socket&& other) noexcept;
    udp_socket& operator=(udp_socket&& other) noexcept;

    ~udp_socket();

    void open();
    void bind(const endpoint& local_endpoint);

    std::size_t send_to(const endpoint& remote_endpoint, std::span<const std::byte> data);
    datagram receive_from(std::size_t max_bytes = 4096);

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;

private:
    native_socket_handle socket_handle_{invalid_native_socket_handle};
};

} // namespace obz::transport
