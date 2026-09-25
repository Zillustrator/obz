#include <obz/transport.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <span>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <vector>

#if defined(__linux__)
#include <cerrno>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace {

std::vector<std::byte> bytes(std::initializer_list<std::uint8_t> values) {
    std::vector<std::byte> result;
    result.reserve(values.size());

    for (const auto value : values) {
        result.push_back(static_cast<std::byte>(value));
    }

    return result;
}

void require_bytes_equal(std::span<const std::byte> actual, std::span<const std::byte> expected) {
    REQUIRE(actual.size() == expected.size());

    for (std::size_t index = 0; index < expected.size(); ++index) {
        REQUIRE(std::to_integer<std::uint8_t>(actual[index]) ==
                std::to_integer<std::uint8_t>(expected[index]));
    }
}

bool is_operation_not_permitted(const std::system_error& error) {
    return error.code() == std::make_error_code(std::errc::operation_not_permitted);
}

} // namespace

TEST_CASE("transport udp_socket sends and receives datagrams on localhost") {
    obz::transport::udp_socket receiver;

    try {
        receiver.bind({"127.0.0.1", 0});
    } catch (const std::system_error& error) {
        if (is_operation_not_permitted(error)) {
            SKIP("localhost UDP bind is not permitted in this environment");
        }

        throw;
    }

    obz::transport::udp_socket sender;
    sender.open();

    const auto payload = bytes({1, 2, 3, 4});

    REQUIRE(sender.send_to(receiver.local_endpoint(), payload) == payload.size());

    const auto datagram = receiver.receive_from(1024);

    require_bytes_equal(datagram.payload, payload);
    REQUIRE(datagram.sender.port != 0);
}

TEST_CASE("transport udp_socket rejects invalid receive size") {
    obz::transport::udp_socket socket;
    socket.open();

    REQUIRE_THROWS_AS(socket.receive_from(0), std::invalid_argument);
}

TEST_CASE("transport tcp_listener accepts a tcp_socket connection on localhost") {
    obz::transport::tcp_listener listener;

    try {
        listener.listen({"127.0.0.1", 0});
    } catch (const std::system_error& error) {
        if (is_operation_not_permitted(error)) {
            SKIP("localhost TCP bind is not permitted in this environment");
        }

        throw;
    }

    const auto listener_endpoint = listener.local_endpoint();
    const auto request = bytes({10, 20, 30});
    const auto response = bytes({40, 50});

    std::array<std::byte, 3> server_received{};
    obz::transport::receive_result server_receive_result{0, obz::transport::receive_status::peer_closed};
    std::size_t server_bytes_sent = 0;
    std::exception_ptr server_error;

    std::thread server([&] {
        try {
            auto socket = listener.accept();

            server_receive_result = socket.receive_exactly(server_received);
            socket.send_all(response);
            server_bytes_sent = response.size();
        } catch (...) {
            server_error = std::current_exception();
        }
    });

    obz::transport::tcp_socket client;
    client.connect(listener_endpoint);

    client.send_all(request);

    std::array<std::byte, 2> client_received{};
    const auto client_receive_result = client.receive_exactly(client_received);

    server.join();

    if (server_error) {
        std::rethrow_exception(server_error);
    }

    REQUIRE(server_bytes_sent == response.size());
    REQUIRE(server_receive_result.status == obz::transport::receive_status::completed);
    REQUIRE(server_receive_result.bytes_received == server_received.size());
    REQUIRE(client_receive_result.status == obz::transport::receive_status::completed);
    REQUIRE(client_receive_result.bytes_received == client_received.size());
    require_bytes_equal(server_received, request);
    require_bytes_equal(client_received, response);
}

TEST_CASE("transport tcp_socket reports peer closure before receiving bytes") {
    obz::transport::tcp_listener listener;
    listener.listen({"127.0.0.1", 0});

    std::exception_ptr server_error;
    std::thread server([&] {
        try {
            const auto socket = listener.accept();
        } catch (...) {
            server_error = std::current_exception();
        }
    });

    obz::transport::tcp_socket client;
    client.connect(listener.local_endpoint());

    std::array<std::byte, 4> destination{};
    const auto result = client.receive_some(destination);

    server.join();

    if (server_error) {
        std::rethrow_exception(server_error);
    }

    REQUIRE(result.status == obz::transport::receive_status::peer_closed);
    REQUIRE(result.bytes_received == 0);
}

TEST_CASE("transport tcp_socket reports bytes received before peer closure") {
    obz::transport::tcp_listener listener;
    listener.listen({"127.0.0.1", 0});

    const auto partial_payload = bytes({1, 2});
    std::exception_ptr server_error;
    std::thread server([&] {
        try {
            auto socket = listener.accept();
            socket.send_all(partial_payload);
        } catch (...) {
            server_error = std::current_exception();
        }
    });

    obz::transport::tcp_socket client;
    client.connect(listener.local_endpoint());

    std::array<std::byte, 4> destination{};
    const auto result = client.receive_exactly(destination);

    server.join();

    if (server_error) {
        std::rethrow_exception(server_error);
    }

    REQUIRE(result.status == obz::transport::receive_status::peer_closed);
    REQUIRE(result.bytes_received == partial_payload.size());
    require_bytes_equal(
        std::span<const std::byte>{destination}.first(result.bytes_received),
        partial_payload);
}

TEST_CASE("transport tcp_socket rejects empty receive destinations") {
    obz::transport::tcp_socket socket;
    std::array<std::byte, 0> destination{};

    REQUIRE_THROWS_AS(socket.receive_some(destination), std::invalid_argument);
    REQUIRE_THROWS_AS(socket.receive_exactly(destination), std::invalid_argument);
}

TEST_CASE("transport tcp_listener rejects invalid backlog") {
    obz::transport::tcp_listener listener;

    REQUIRE_THROWS_AS(listener.listen({"127.0.0.1", 0}, 0), std::invalid_argument);
}

TEST_CASE("transport sockets report open state and close idempotently") {
    obz::transport::udp_socket socket;

    REQUIRE_FALSE(socket.is_open());

    socket.open();

    REQUIRE(socket.is_open());
    REQUIRE(socket.native_handle() != obz::transport::invalid_native_socket_handle);

    socket.close();
    socket.close();

    REQUIRE_FALSE(socket.is_open());
}

TEST_CASE("transport tcp_socket send_all rejects unopened sockets") {
    obz::transport::tcp_socket socket;
    const auto payload = bytes({1, 2, 3});

    REQUIRE_THROWS_AS(socket.send_all(payload), std::runtime_error);
}

#if defined(__linux__)
TEST_CASE("transport tcp_socket reports a broken send without terminating the process") {
    const auto child = ::fork();

    REQUIRE(child >= 0);

    if (child == 0) {
        int handles[2]{};

        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, handles) != 0) {
            ::_exit(1);
        }

        ::close(handles[1]);

        obz::transport::tcp_socket socket{handles[0]};
        const std::array payload{std::byte{1}};

        try {
            static_cast<void>(socket.send(payload));
        } catch (const std::system_error& error) {
            ::_exit(error.code().value() == EPIPE ? 0 : 2);
        } catch (...) {
            ::_exit(3);
        }

        ::_exit(4);
    }

    int child_status{};
    REQUIRE(::waitpid(child, &child_status, 0) == child);
    REQUIRE(WIFEXITED(child_status));
    REQUIRE(WEXITSTATUS(child_status) == 0);
}
#endif
