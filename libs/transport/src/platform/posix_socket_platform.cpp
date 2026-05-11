#include "../socket_platform.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <stdexcept>
#include <utility>

namespace obz::transport::detail {

namespace {

sockaddr_in to_sockaddr_in(const endpoint& value) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(value.port);

    const auto result = ::inet_pton(AF_INET, value.host.c_str(), &address.sin_addr);

    if (result != 1) {
        throw std::invalid_argument("invalid IPv4 endpoint host: " + value.host);
    }

    return address;
}

endpoint from_sockaddr_in(const sockaddr_in& address) {
    char host[INET_ADDRSTRLEN]{};

    if (::inet_ntop(AF_INET, &address.sin_addr, host, sizeof(host)) == nullptr) {
        throw last_socket_error("failed to convert IPv4 address to text");
    }

    return endpoint{std::string(host), ntohs(address.sin_port)};
}

} // namespace

native_socket_handle invalid_socket() noexcept {
    return invalid_native_socket_handle;
}

bool is_valid(native_socket_handle handle) noexcept {
    return handle != invalid_socket();
}

void close_socket(native_socket_handle handle) noexcept {
    if (is_valid(handle)) {
        ::close(handle);
    }
}

std::system_error last_socket_error(const std::string& message) {
    return std::system_error(errno, std::generic_category(), message);
}

native_socket_handle create_tcp_socket() {
    const auto handle = ::socket(AF_INET, SOCK_STREAM, 0);

    if (!is_valid(handle)) {
        throw last_socket_error("failed to create TCP socket");
    }

    return handle;
}

native_socket_handle create_udp_socket() {
    const auto handle = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (!is_valid(handle)) {
        throw last_socket_error("failed to create UDP socket");
    }

    return handle;
}

void connect_socket(native_socket_handle handle, const endpoint& remote_endpoint) {
    const auto address = to_sockaddr_in(remote_endpoint);

    if (::connect(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) {
        throw last_socket_error(
            "failed to connect to " + remote_endpoint.host + ":" +
            std::to_string(remote_endpoint.port));
    }
}

void bind_socket(native_socket_handle handle, const endpoint& local_endpoint) {
    const auto address = to_sockaddr_in(local_endpoint);

    if (::bind(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) {
        throw last_socket_error(
            "failed to bind socket to " + local_endpoint.host + ":" +
            std::to_string(local_endpoint.port));
    }
}

void set_reuse_address(native_socket_handle handle) {
    int reuse_address = 1;

    if (::setsockopt(
            handle,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse_address,
            sizeof(reuse_address)) < 0) {
        throw last_socket_error("failed to set SO_REUSEADDR on TCP listener");
    }
}

void listen_socket(native_socket_handle handle, int backlog) {
    if (::listen(handle, backlog) < 0) {
        throw last_socket_error("failed to listen on TCP socket");
    }
}

native_socket_handle accept_socket(native_socket_handle handle) {
    const auto client_handle = ::accept(handle, nullptr, nullptr);

    if (!is_valid(client_handle)) {
        throw last_socket_error("failed to accept TCP connection");
    }

    return client_handle;
}

std::size_t send_tcp(native_socket_handle handle, std::span<const std::byte> data) {
    const auto bytes_sent = ::send(handle, data.data(), data.size(), 0);

    if (bytes_sent < 0) {
        throw last_socket_error("failed to send TCP data");
    }

    return static_cast<std::size_t>(bytes_sent);
}

std::vector<std::byte> receive_tcp(native_socket_handle handle, std::size_t max_bytes) {
    std::vector<std::byte> buffer(max_bytes);
    const auto bytes_received = ::recv(handle, buffer.data(), buffer.size(), 0);

    if (bytes_received < 0) {
        throw last_socket_error("failed to receive TCP data");
    }

    buffer.resize(static_cast<std::size_t>(bytes_received));
    return buffer;
}

std::size_t send_udp(
    native_socket_handle handle,
    const endpoint& remote_endpoint,
    std::span<const std::byte> data) {
    const auto address = to_sockaddr_in(remote_endpoint);
    const auto bytes_sent = ::sendto(
        handle,
        data.data(),
        data.size(),
        0,
        reinterpret_cast<const sockaddr*>(&address),
        sizeof(address));

    if (bytes_sent < 0) {
        throw last_socket_error("failed to send UDP datagram");
    }

    return static_cast<std::size_t>(bytes_sent);
}

datagram receive_udp(native_socket_handle handle, std::size_t max_bytes) {
    std::vector<std::byte> buffer(max_bytes);
    sockaddr_in sender_address{};
    socklen_t sender_address_size = sizeof(sender_address);

    const auto bytes_received = ::recvfrom(
        handle,
        buffer.data(),
        buffer.size(),
        0,
        reinterpret_cast<sockaddr*>(&sender_address),
        &sender_address_size);

    if (bytes_received < 0) {
        throw last_socket_error("failed to receive UDP datagram");
    }

    buffer.resize(static_cast<std::size_t>(bytes_received));
    return datagram{from_sockaddr_in(sender_address), std::move(buffer)};
}

endpoint local_endpoint_for(native_socket_handle handle) {
    sockaddr_in address{};
    socklen_t address_size = sizeof(address);

    if (::getsockname(handle, reinterpret_cast<sockaddr*>(&address), &address_size) < 0) {
        throw last_socket_error("failed to read local socket endpoint");
    }

    return from_sockaddr_in(address);
}

} // namespace obz::transport::detail
