#include <obz/transport/tcp_listener.hpp>

#include "socket_platform.hpp"

#include <stdexcept>
#include <utility>

namespace obz::transport {

tcp_listener::tcp_listener(tcp_listener&& other) noexcept
    : listen_handle_(other.listen_handle_) {
    other.listen_handle_ = detail::invalid_socket();
}

tcp_listener& tcp_listener::operator=(tcp_listener&& other) noexcept {
    if (this != &other) {
        close();
        listen_handle_ = other.listen_handle_;
        other.listen_handle_ = detail::invalid_socket();
    }

    return *this;
}

tcp_listener::~tcp_listener() {
    close();
}

void tcp_listener::listen(const endpoint& local_endpoint, int backlog) {
    if (backlog <= 0) {
        throw std::invalid_argument("TCP listener backlog must be greater than zero");
    }

    close();

    listen_handle_ = detail::create_tcp_socket();

    try {
        detail::set_reuse_address(listen_handle_);
        detail::bind_socket(listen_handle_, local_endpoint);
        detail::listen_socket(listen_handle_, backlog);
    } catch (...) {
        close();
        throw;
    }
}

tcp_socket tcp_listener::accept() {
    if (!is_open()) {
        throw std::runtime_error("TCP listener is not open");
    }

    return tcp_socket(detail::accept_socket(listen_handle_));
}

void tcp_listener::close() {
    if (is_open()) {
        detail::close_socket(listen_handle_);
        listen_handle_ = detail::invalid_socket();
    }
}

bool tcp_listener::is_open() const {
    return detail::is_valid(listen_handle_);
}

native_socket_handle tcp_listener::native_handle() const {
    return listen_handle_;
}

endpoint tcp_listener::local_endpoint() const {
    if (!is_open()) {
        throw std::runtime_error("TCP listener is not open");
    }

    return detail::local_endpoint_for(listen_handle_);
}

} // namespace obz::transport
