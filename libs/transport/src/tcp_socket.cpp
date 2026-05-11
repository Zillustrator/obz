#include <obz/transport/tcp_socket.hpp>

#include "socket_platform.hpp"

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

std::vector<std::byte> tcp_socket::receive(std::size_t max_bytes) {
    if (!is_open()) {
        throw std::runtime_error("TCP socket is not open");
    }

    if (max_bytes == 0) {
        return {};
    }

    return detail::receive_tcp(socket_handle_, max_bytes);
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
