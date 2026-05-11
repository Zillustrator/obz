#include <obz/transport/udp_socket.hpp>

#include "socket_platform.hpp"

#include <stdexcept>
#include <utility>

namespace obz::transport {

udp_socket::udp_socket(udp_socket&& other) noexcept
    : socket_handle_(other.socket_handle_) {
    other.socket_handle_ = detail::invalid_socket();
}

udp_socket& udp_socket::operator=(udp_socket&& other) noexcept {
    if (this != &other) {
        close();
        socket_handle_ = other.socket_handle_;
        other.socket_handle_ = detail::invalid_socket();
    }

    return *this;
}

udp_socket::~udp_socket() {
    close();
}

void udp_socket::open() {
    if (is_open()) {
        throw std::runtime_error("UDP socket is already open");
    }

    socket_handle_ = detail::create_udp_socket();
}

void udp_socket::bind(const endpoint& local_endpoint) {
    close();
    open();

    try {
        detail::bind_socket(socket_handle_, local_endpoint);
    } catch (...) {
        close();
        throw;
    }
}

std::size_t udp_socket::send_to(const endpoint& remote_endpoint, std::span<const std::byte> data) {
    if (!is_open()) {
        throw std::runtime_error("UDP socket is not open");
    }

    return detail::send_udp(socket_handle_, remote_endpoint, data);
}

datagram udp_socket::receive_from(std::size_t max_bytes) {
    if (!is_open()) {
        throw std::runtime_error("UDP socket is not open");
    }

    if (max_bytes == 0) {
        throw std::invalid_argument("UDP receive size must be greater than zero");
    }

    return detail::receive_udp(socket_handle_, max_bytes);
}

void udp_socket::close() {
    if (is_open()) {
        detail::close_socket(socket_handle_);
        socket_handle_ = detail::invalid_socket();
    }
}

bool udp_socket::is_open() const {
    return detail::is_valid(socket_handle_);
}

native_socket_handle udp_socket::native_handle() const {
    return socket_handle_;
}

endpoint udp_socket::local_endpoint() const {
    if (!is_open()) {
        throw std::runtime_error("UDP socket is not open");
    }

    return detail::local_endpoint_for(socket_handle_);
}

} // namespace obz::transport
