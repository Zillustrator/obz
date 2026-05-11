# obz::transport

Small RAII wrappers for TCP and UDP sockets.

This library solves one narrow problem: moving bytes between IPv4 endpoints while keeping native socket handles owned, closed, and difficult to misuse.

---

## Features

- IPv4 endpoint type
- Move-only TCP socket wrapper
- TCP listener with `accept`
- UDP socket with `send_to` and `receive_from`
- RAII close in destructors
- `std::system_error` for operating-system socket failures
- Byte-oriented APIs using `std::byte` and `std::span`
- POSIX backend, with a Windows Winsock backend selected by CMake on Windows

---

## Usage

```cpp
#include <obz/transport.hpp>

obz::transport::udp_socket receiver;
receiver.bind({"127.0.0.1", 9000});
```

---

## UDP Example

```cpp
#include <obz/transport.hpp>

#include <array>

int main() {
    obz::transport::udp_socket receiver;
    receiver.bind({"127.0.0.1", 9000});

    obz::transport::udp_socket sender;
    sender.open();

    std::array<std::byte, 3> payload{
        std::byte{1},
        std::byte{2},
        std::byte{3},
    };

    sender.send_to({"127.0.0.1", 9000}, payload);

    const auto datagram = receiver.receive_from();
}
```

---

## API

### endpoint

```cpp
struct endpoint {
    std::string host;
    std::uint16_t port;
};
```

Represents an IPv4 endpoint.

---

### udp_socket

```cpp
class udp_socket {
public:
    void open();
    void bind(const endpoint& local_endpoint);

    std::size_t send_to(const endpoint& remote_endpoint, std::span<const std::byte> data);
    datagram receive_from(std::size_t max_bytes = 4096);

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;
};
```

`receive_from` blocks until one datagram is received.

---

### tcp_socket

```cpp
class tcp_socket {
public:
    void connect(const endpoint& remote_endpoint);

    std::size_t send(std::span<const std::byte> data);
    std::vector<std::byte> receive(std::size_t max_bytes = 4096);

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;
};
```

`send` and `receive` are byte-oriented wrappers over native socket operations. A single `send` may write fewer bytes than requested.

---

### tcp_listener

```cpp
class tcp_listener {
public:
    void listen(const endpoint& local_endpoint, int backlog = 8);
    tcp_socket accept();

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;
};
```

`accept` blocks until a client connects.

---

## Behaviour Summary

| Operation | Invalid State | OS Failure |
|-----------|---------------|------------|
| socket operation before open | throws `std::runtime_error` | n/a |
| bind/connect/send/receive/listen/accept | n/a | throws `std::system_error` |
| close | idempotent | ignored |

---

## Threading Model

The socket wrappers are not internally synchronized.

Use each socket from one thread at a time, or provide external synchronization around shared socket objects.

---

## Design Notes

The classes are move-only because each object owns one native socket handle.

The public API uses `native_socket_handle` instead of exposing POSIX file descriptors directly. On POSIX the handle is an `int`; on Windows it is represented by a pointer-sized unsigned integer compatible with Winsock `SOCKET` values.

`local_endpoint()` is provided so callers and tests can bind to port `0` and discover the actual ephemeral port chosen by the operating system.

UDP uses `udp_socket`, not `udp_listener`, because UDP is connectionless and does not accept client connections.

---

## Platform Support

The public socket classes are shared across platforms. Platform-specific socket details live behind a small internal implementation boundary:

- POSIX builds compile `src/platform/posix_socket_platform.cpp`
- Windows builds compile `src/platform/win32_socket_platform.cpp` and link `ws2_32`

The platform layer owns native socket creation, close semantics, address conversion, error conversion, and Winsock startup on Windows.

This keeps the user-facing API stable while letting CMake select the platform backend.

The POSIX backend is covered by the current local build. The Windows backend follows the same internal boundary and should be validated on Windows before treating it as production-ready.

---

## When to Use

Use `transport` when:

- you need small socket wrappers without bringing in a networking framework
- localhost tests need real TCP or UDP sockets
- you want byte-oriented APIs with RAII ownership
- you want a minimal example of separating portable socket classes from platform-specific socket calls

---

## When Not to Use

Avoid `transport` when:

- asynchronous I/O or event loops are required
- TLS is required
- DNS resolution is required
- IPv6 support is required

---

## Future Improvements

Potential extensions:

- non-blocking mode
- timeout configuration
- DNS resolution helpers
- IPv6 endpoints
- send-all helper for TCP streams

---

## License

Part of the `obz` project.
