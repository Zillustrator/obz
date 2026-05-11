#include <obz/transport.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <vector>

namespace {

std::vector<std::byte> bytes(std::initializer_list<std::uint8_t> values) {
    std::vector<std::byte> result;
    result.reserve(values.size());

    for (const auto value : values) {
        result.push_back(static_cast<std::byte>(value));
    }

    return result;
}

void require_bytes_equal(const std::vector<std::byte>& actual, const std::vector<std::byte>& expected) {
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

    std::vector<std::byte> server_received;
    std::size_t server_bytes_sent = 0;
    std::exception_ptr server_error;

    std::thread server([&] {
        try {
            auto socket = listener.accept();

            server_received = socket.receive(1024);
            server_bytes_sent = socket.send(response);
        } catch (...) {
            server_error = std::current_exception();
        }
    });

    obz::transport::tcp_socket client;
    client.connect(listener_endpoint);

    REQUIRE(client.send(request) == request.size());

    const auto client_received = client.receive(1024);

    server.join();

    if (server_error) {
        std::rethrow_exception(server_error);
    }

    REQUIRE(server_bytes_sent == response.size());
    require_bytes_equal(server_received, request);
    require_bytes_equal(client_received, response);
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
