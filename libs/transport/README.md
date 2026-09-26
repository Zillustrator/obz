# obz::transport

Small RAII wrappers for TCP and UDP sockets.

This library solves one narrow problem: moving bytes between IPv4 endpoints while keeping native socket handles owned, closed, and difficult to misuse.

---

## Features

- IPv4 endpoint type
- Move-only TCP socket wrapper
- TCP listener with `accept`
- UDP socket with `send_to` and `receive_from`
- Explicit reporting of complete, empty and truncated UDP datagrams
- TCP `send_all` helper for complete buffer writes
- TCP `receive_some` and `receive_exactly` operations using caller-owned storage
- RAII close in destructors
- `std::system_error` for operating-system socket failures
- Broken TCP sends report errors without allowing POSIX `SIGPIPE` to terminate the process
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

    std::array<std::byte, 4096> receive_buffer{};
    const auto result = receiver.receive_from(receive_buffer);

    if (result.status == obz::transport::datagram_status::truncated) {
        return 1;
    }

    const auto received =
        std::span<const std::byte>{receive_buffer}.first(result.bytes_received);

    // Decode only `received`, not the unused remainder of `receive_buffer`.
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
enum class datagram_status {
    complete,
    truncated,
};

struct udp_receive_result {
    endpoint sender;
    std::size_t bytes_received;
    datagram_status status;
};

class udp_socket {
public:
    void open();
    void bind(const endpoint& local_endpoint);

    std::size_t send_to(const endpoint& remote_endpoint, std::span<const std::byte> data);
    [[nodiscard]] udp_receive_result receive_from(std::span<std::byte> destination);

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;
};
```

`receive_from` blocks until one datagram is received and writes into caller-owned storage. A
complete result may contain zero bytes because UDP permits empty datagrams. If the datagram is
larger than the destination, the result is `truncated`, `bytes_received` equals the bytes retained
in the destination, and the remainder of that datagram has been discarded by the operating system.
The next receive starts with the next datagram.

---

### tcp_socket

```cpp
enum class receive_status {
    completed,
    peer_closed,
};

struct receive_result {
    std::size_t bytes_received{0};
    receive_status status{receive_status::peer_closed};
};

class tcp_socket {
public:
    void connect(const endpoint& remote_endpoint);

    std::size_t send(std::span<const std::byte> data);
    void send_all(std::span<const std::byte> data);
    [[nodiscard]] receive_result receive_some(std::span<std::byte> destination);
    [[nodiscard]] receive_result receive_exactly(std::span<std::byte> destination);

    void close();

    bool is_open() const;
    native_socket_handle native_handle() const;
    endpoint local_endpoint() const;
};
```

`send` and `receive_some` are byte-oriented wrappers over native socket operations. A single call may transfer fewer bytes than the supplied span contains.

`send_all` repeatedly calls `send` until the full span has been written, or throws if the socket reports failure.

Sending after a connection has broken throws `std::system_error`. On Linux the backend uses `MSG_NOSIGNAL` for each TCP send. On macOS it configures created and accepted TCP sockets with `SO_NOSIGPIPE`. This prevents the default `SIGPIPE` action from terminating the process without changing the application's process-wide signal policy.

`receive_some` performs one blocking receive into caller-owned storage. A completed result contains a positive byte count no greater than the destination size. `peer_closed` contains a zero byte count.

`receive_exactly` repeatedly receives until it fills the destination. If the peer closes first, its result reports `peer_closed` and the number of bytes placed in the destination before closure.

Both receive operations reject an empty destination with `std::invalid_argument`. Native socket failures throw `std::system_error`.

On POSIX, an interrupted blocking operation is reported as `std::system_error` with `std::errc::interrupted`. The library does not retry automatically because a signal may be the application's mechanism for requesting shutdown. Callers can choose whether retrying is correct for their operation and shutdown policy.

```cpp
std::array<std::byte, 8> header_bytes{};
const auto result = socket.receive_exactly(header_bytes);

if (result.status == obz::transport::receive_status::peer_closed) {
    if (result.bytes_received == 0) {
        // The peer closed before the next header began.
    } else {
        // The peer closed partway through the header.
    }
}
```

The transport layer reports closure and transferred bytes without assigning protocol meaning. A framing layer can treat closure before a new header as a normal end of stream and closure within a header or payload as an unexpected end of a message.

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

| Situation | Result |
|-----------|--------|
| socket operation before open | throws `std::runtime_error` |
| bind/connect/send/receive/listen/accept OS failure | throws `std::system_error` |
| TCP or UDP receive with an empty destination | throws `std::invalid_argument` |
| TCP peer closes during receive | returns `receive_status::peer_closed` |
| complete empty UDP datagram | returns `datagram_status::complete` with zero bytes |
| UDP datagram exceeds the destination | returns `datagram_status::truncated` with the retained byte count |
| close | idempotent; native close failures are ignored |

---

## Threading Model

The socket wrappers are not internally synchronized.

Use each socket from one thread at a time, or provide external synchronization around shared socket objects.

---

## Design Notes

The classes are move-only because each object owns one native socket handle.

The public API uses `native_socket_handle` instead of exposing POSIX file descriptors directly. On POSIX the handle is an `int`; on Windows it is represented by a pointer-sized unsigned integer compatible with Winsock `SOCKET` values.

Receive operations use caller-owned spans so callers can choose and reuse storage without requiring the socket to own a buffer or allocate on every call. TCP results make orderly peer closure explicit, while UDP results report the sender, retained byte count and truncation status.

`local_endpoint()` is provided so callers and tests can bind to port `0` and discover the actual ephemeral port chosen by the operating system.

UDP uses `udp_socket`, not `udp_listener`, because UDP is connectionless and does not accept client connections.

---

## Platform Support

The public socket classes are shared across platforms. Platform-specific socket details live behind a small internal implementation boundary:

- POSIX builds compile `src/platform/posix_socket_platform.cpp`
- Windows builds compile `src/platform/win32_socket_platform.cpp` and link `ws2_32`

The platform layer owns native socket creation, close semantics, address conversion, error conversion, and Winsock startup on Windows.

Each backend captures `errno` or `WSAGetLastError()` immediately after a failed native call, before constructing messages or performing cleanup that could replace the original error code.

`close()` remains idempotent and non-throwing. The POSIX backend does not retry an interrupted `close()` because the descriptor may already have been released and reused for another resource.

Winsock reports broken sends through its normal error return and does not use `SIGPIPE`.

This keeps the user-facing API stable while letting CMake select the platform backend.

The POSIX backend is tested on macOS and Linux. The Windows workflow builds with
MSVC on Windows Server 2022 and runs the same portable suite, including localhost
TCP and UDP integration tests against Winsock. Platform-specific POSIX tests for
interrupted system calls and Linux `SIGPIPE` handling run only where those APIs
exist.

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
- vectored send/receive helpers for TCP streams

---

## License

Part of the `obz` project.
