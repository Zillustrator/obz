#include <obz/transport/tcp_socket.hpp>

#include "socket_platform.hpp"

#include <stdexcept>
#include <utility>

namespace obz::transport {

tcp_socket::tcp_socket(native_socket_handle socket_handle)
    : socket_handle_(socket_handle) {}

tcp_socket::tcp_socket(tcp_socket&& other) noexcept
    : socket_handle_(other.socket_handle_) {
    other.socket_handle_ = detail::invalid_socket();
}

tcp_socket& tcp_socket::operator=(tcp_socket&& other) noexcept {
    if (this != &other) {
        close();
        socket_handle_ = other.socket_handle_;
        other.socket_handle_ = detail::invalid_socket();
    }

    return *this;
}

tcp_socket::~tcp_socket() {
    close();
}

void tcp_socket::connect(const endpoint& remote_endpoint) {
    close();

    socket_handle_ = detail::create_tcp_socket();

    try {
        detail::connect_socket(socket_handle_, remote_endpoint);
    } catch (...) {
        close();
        throw;
    }
}

std::size_t tcp_socket::send(std::span<const std::byte> data) {
    if (!is_open()) {
        throw std::runtime_error("TCP socket is not open");
    }

    return detail::send_tcp(socket_handle_, data);
}

void tcp_socket::send_all(std::span<const std::byte> data) {
    std::size_t bytes_sent = 0;

    while (bytes_sent < data.size()) {
        const auto remaining = data.subspan(bytes_sent);
        const auto sent = send(remaining);

        if (sent == 0) {
            throw std::runtime_error("TCP socket send wrote zero bytes");
        }

        bytes_sent += sent;
    }
}

receive_result tcp_socket::receive_some(std::span<std::byte> destination) {
    if (destination.empty()) {
        throw std::invalid_argument("TCP receive destination must not be empty");
    }

    if (!is_open()) {
        throw std::runtime_error("TCP socket is not open");
    }

    const auto bytes_received = detail::receive_tcp(socket_handle_, destination);

    if (bytes_received == 0) {
        return receive_result{0, receive_status::peer_closed};
    }

    return receive_result{bytes_received, receive_status::completed};
}

receive_result tcp_socket::receive_exactly(std::span<std::byte> destination) {
    if (destination.empty()) {
        throw std::invalid_argument("TCP receive destination must not be empty");
    }

    std::size_t total_received = 0;

    while (total_received < destination.size()) {
        const auto result = receive_some(destination.subspan(total_received));

        if (result.status == receive_status::peer_closed) {
            return receive_result{total_received, receive_status::peer_closed};
        }

        total_received += result.bytes_received;
    }

    return receive_result{total_received, receive_status::completed};
}

void tcp_socket::close() {
    if (is_open()) {
        detail::close_socket(socket_handle_);
        socket_handle_ = detail::invalid_socket();
    }
}

bool tcp_socket::is_open() const {
    return detail::is_valid(socket_handle_);
}

native_socket_handle tcp_socket::native_handle() const {
    return socket_handle_;
}

endpoint tcp_socket::local_endpoint() const {
    if (!is_open()) {
        throw std::runtime_error("TCP socket is not open");
    }

    return detail::local_endpoint_for(socket_handle_);
}

} // namespace obz::transport
