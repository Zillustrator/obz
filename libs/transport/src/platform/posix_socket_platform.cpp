#include "../socket_platform.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace obz::transport::detail {

namespace {

std::system_error socket_error(int error_code, const std::string& message) {
    return std::system_error(error_code, std::generic_category(), message);
}

std::system_error last_socket_error(const char* message) {
    const auto error_code = errno;
    return socket_error(error_code, message);
}

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

ip_mreq multicast_request(
    const std::string& group_address, const std::string& interface_address) {
    // Reject embedded NULs before passing strings to native parsers.
    if (group_address.find('\0') != std::string::npos ||
        interface_address.find('\0') != std::string::npos) {
        throw std::invalid_argument("multicast addresses must not contain embedded NULs");
    }

    ip_mreq request{};
    request.imr_multiaddr = to_sockaddr_in({group_address, 0}).sin_addr;
    request.imr_interface = to_sockaddr_in({interface_address, 0}).sin_addr;

    const auto group = ntohl(request.imr_multiaddr.s_addr);
    if ((group & 0xf0000000u) != 0xe0000000u) {
        throw std::invalid_argument("group address must be an IPv4 multicast address");
    }

    const auto interface_host = ntohl(request.imr_interface.s_addr);
    if (interface_host != 0 &&
        ((interface_host & 0xff000000u) == 0 || interface_host >= 0xe0000000u)) {
        throw std::invalid_argument("interface address must be a local unicast IPv4 address or 0.0.0.0");
    }

    return request;
}

endpoint from_sockaddr_in(const sockaddr_in& address) {
    char host[INET_ADDRSTRLEN]{};

    if (::inet_ntop(AF_INET, &address.sin_addr, host, sizeof(host)) == nullptr) {
        throw last_socket_error("failed to convert IPv4 address to text");
    }

    return endpoint{std::string(host), ntohs(address.sin_port)};
}

native_socket_handle configure_tcp_socket(native_socket_handle handle) {
#if defined(__APPLE__)
    int no_sigpipe = 1;

    if (::setsockopt(handle, SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe)) < 0) {
        const auto error = last_socket_error("failed to disable SIGPIPE on TCP socket");
        ::close(handle);
        throw error;
    }
#endif

    return handle;
}

int tcp_send_flags() noexcept {
#if defined(MSG_NOSIGNAL)
    return MSG_NOSIGNAL;
#else
    return 0;
#endif
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

native_socket_handle create_tcp_socket() {
    const auto handle = ::socket(AF_INET, SOCK_STREAM, 0);

    if (!is_valid(handle)) {
        throw last_socket_error("failed to create TCP socket");
    }

    return configure_tcp_socket(handle);
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
        const auto error_code = errno;
        throw socket_error(
            error_code,
            "failed to connect to " + remote_endpoint.host + ":" +
            std::to_string(remote_endpoint.port));
    }
}

void bind_socket(native_socket_handle handle, const endpoint& local_endpoint) {
    const auto address = to_sockaddr_in(local_endpoint);

    if (::bind(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) {
        const auto error_code = errno;
        throw socket_error(
            error_code,
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

    return configure_tcp_socket(client_handle);
}

std::size_t send_tcp(native_socket_handle handle, std::span<const std::byte> data) {
    const auto bytes_sent = ::send(handle, data.data(), data.size(), tcp_send_flags());

    if (bytes_sent < 0) {
        throw last_socket_error("failed to send TCP data");
    }

    return static_cast<std::size_t>(bytes_sent);
}

std::size_t receive_tcp(native_socket_handle handle, std::span<std::byte> destination) {
    const auto bytes_received = ::recv(handle, destination.data(), destination.size(), 0);

    if (bytes_received < 0) {
        throw last_socket_error("failed to receive TCP data");
    }

    return static_cast<std::size_t>(bytes_received);
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

udp_receive_result receive_udp(
    native_socket_handle handle,
    std::span<std::byte> destination) {
    sockaddr_in sender_address{};
    iovec buffer{destination.data(), destination.size()};
    msghdr message{};
    message.msg_name = &sender_address;
    message.msg_namelen = sizeof(sender_address);
    message.msg_iov = &buffer;
    message.msg_iovlen = 1;

    const auto bytes_received = ::recvmsg(handle, &message, 0);

    if (bytes_received < 0) {
        throw last_socket_error("failed to receive UDP datagram");
    }

    const auto status = (message.msg_flags & MSG_TRUNC) != 0
                            ? datagram_status::truncated
                            : datagram_status::complete;

    return udp_receive_result{
        from_sockaddr_in(sender_address),
        static_cast<std::size_t>(bytes_received),
        status};
}

void join_multicast_group(
    native_socket_handle handle,
    const std::string& group_address,
    const std::string& interface_address) {
    const auto request = multicast_request(group_address, interface_address);

    if (::setsockopt(handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, &request, sizeof(request)) < 0) {
        throw last_socket_error("failed to join IPv4 multicast group");
    }
}

void leave_multicast_group(
    native_socket_handle handle,
    const std::string& group_address,
    const std::string& interface_address) {
    const auto request = multicast_request(group_address, interface_address);

    if (::setsockopt(handle, IPPROTO_IP, IP_DROP_MEMBERSHIP, &request, sizeof(request)) < 0) {
        throw last_socket_error("failed to leave IPv4 multicast group");
    }
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
