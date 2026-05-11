#include "../socket_platform.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdint>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace obz::transport::detail {

namespace {

SOCKET to_socket(native_socket_handle handle) noexcept {
    return static_cast<SOCKET>(handle);
}

native_socket_handle from_socket(SOCKET socket) noexcept {
    return static_cast<native_socket_handle>(socket);
}

void ensure_winsock_started() {
    static std::once_flag once;
    static int startup_result = 0;

    std::call_once(once, [] {
        WSADATA data{};
        startup_result = ::WSAStartup(MAKEWORD(2, 2), &data);
    });

    if (startup_result != 0) {
        throw std::system_error(
            startup_result,
            std::system_category(),
            "failed to start Winsock");
    }
}

sockaddr_in to_sockaddr_in(const endpoint& value) {
    ensure_winsock_started();

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(value.port);

    const auto result = ::InetPtonA(AF_INET, value.host.c_str(), &address.sin_addr);

    if (result != 1) {
        throw std::invalid_argument("invalid IPv4 endpoint host: " + value.host);
    }

    return address;
}

endpoint from_sockaddr_in(const sockaddr_in& address) {
    char host[INET_ADDRSTRLEN]{};

    if (::InetNtopA(AF_INET, const_cast<in_addr*>(&address.sin_addr), host, sizeof(host)) == nullptr) {
        throw last_socket_error("failed to convert IPv4 address to text");
    }

    return endpoint{std::string(host), ntohs(address.sin_port)};
}

int checked_socket_size(std::size_t size) {
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("socket operation size exceeds Winsock limit");
    }

    return static_cast<int>(size);
}

} // namespace

native_socket_handle invalid_socket() noexcept {
    return invalid_native_socket_handle;
}

bool is_valid(native_socket_handle handle) noexcept {
    return to_socket(handle) != INVALID_SOCKET;
}

void close_socket(native_socket_handle handle) noexcept {
    if (is_valid(handle)) {
        ::closesocket(to_socket(handle));
    }
}

std::system_error last_socket_error(const std::string& message) {
    return std::system_error(::WSAGetLastError(), std::system_category(), message);
}

native_socket_handle create_tcp_socket() {
    ensure_winsock_started();

    const auto socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socket == INVALID_SOCKET) {
        throw last_socket_error("failed to create TCP socket");
    }

    return from_socket(socket);
}

native_socket_handle create_udp_socket() {
    ensure_winsock_started();

    const auto socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (socket == INVALID_SOCKET) {
        throw last_socket_error("failed to create UDP socket");
    }

    return from_socket(socket);
}

void connect_socket(native_socket_handle handle, const endpoint& remote_endpoint) {
    const auto address = to_sockaddr_in(remote_endpoint);

    if (::connect(to_socket(handle), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) ==
        SOCKET_ERROR) {
        throw last_socket_error(
            "failed to connect to " + remote_endpoint.host + ":" +
            std::to_string(remote_endpoint.port));
    }
}

void bind_socket(native_socket_handle handle, const endpoint& local_endpoint) {
    const auto address = to_sockaddr_in(local_endpoint);

    if (::bind(to_socket(handle), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) ==
        SOCKET_ERROR) {
        throw last_socket_error(
            "failed to bind socket to " + local_endpoint.host + ":" +
            std::to_string(local_endpoint.port));
    }
}

void set_reuse_address(native_socket_handle handle) {
    BOOL reuse_address = TRUE;

    if (::setsockopt(
            to_socket(handle),
            SOL_SOCKET,
            SO_REUSEADDR,
            reinterpret_cast<const char*>(&reuse_address),
            sizeof(reuse_address)) == SOCKET_ERROR) {
        throw last_socket_error("failed to set SO_REUSEADDR on TCP listener");
    }
}

void listen_socket(native_socket_handle handle, int backlog) {
    if (::listen(to_socket(handle), backlog) == SOCKET_ERROR) {
        throw last_socket_error("failed to listen on TCP socket");
    }
}

native_socket_handle accept_socket(native_socket_handle handle) {
    const auto client_socket = ::accept(to_socket(handle), nullptr, nullptr);

    if (client_socket == INVALID_SOCKET) {
        throw last_socket_error("failed to accept TCP connection");
    }

    return from_socket(client_socket);
}

std::size_t send_tcp(native_socket_handle handle, std::span<const std::byte> data) {
    const auto bytes_sent = ::send(
        to_socket(handle),
        reinterpret_cast<const char*>(data.data()),
        checked_socket_size(data.size()),
        0);

    if (bytes_sent == SOCKET_ERROR) {
        throw last_socket_error("failed to send TCP data");
    }

    return static_cast<std::size_t>(bytes_sent);
}

std::vector<std::byte> receive_tcp(native_socket_handle handle, std::size_t max_bytes) {
    std::vector<std::byte> buffer(max_bytes);
    const auto bytes_received = ::recv(
        to_socket(handle),
        reinterpret_cast<char*>(buffer.data()),
        checked_socket_size(buffer.size()),
        0);

    if (bytes_received == SOCKET_ERROR) {
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
        to_socket(handle),
        reinterpret_cast<const char*>(data.data()),
        checked_socket_size(data.size()),
        0,
        reinterpret_cast<const sockaddr*>(&address),
        sizeof(address));

    if (bytes_sent == SOCKET_ERROR) {
        throw last_socket_error("failed to send UDP datagram");
    }

    return static_cast<std::size_t>(bytes_sent);
}

datagram receive_udp(native_socket_handle handle, std::size_t max_bytes) {
    std::vector<std::byte> buffer(max_bytes);
    sockaddr_in sender_address{};
    int sender_address_size = sizeof(sender_address);

    const auto bytes_received = ::recvfrom(
        to_socket(handle),
        reinterpret_cast<char*>(buffer.data()),
        checked_socket_size(buffer.size()),
        0,
        reinterpret_cast<sockaddr*>(&sender_address),
        &sender_address_size);

    if (bytes_received == SOCKET_ERROR) {
        throw last_socket_error("failed to receive UDP datagram");
    }

    buffer.resize(static_cast<std::size_t>(bytes_received));
    return datagram{from_sockaddr_in(sender_address), std::move(buffer)};
}

endpoint local_endpoint_for(native_socket_handle handle) {
    sockaddr_in address{};
    int address_size = sizeof(address);

    if (::getsockname(to_socket(handle), reinterpret_cast<sockaddr*>(&address), &address_size) ==
        SOCKET_ERROR) {
        throw last_socket_error("failed to read local socket endpoint");
    }

    return from_sockaddr_in(address);
}

} // namespace obz::transport::detail
