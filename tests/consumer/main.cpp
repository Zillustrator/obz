#include <obz/blocking_queue.hpp>
#include <obz/spsc_ring_queue.hpp>
#include <obz/mpsc_ring_queue.hpp>
#include <obz/endian.hpp>
#include <obz/thread_affinity.hpp>
#include <obz/transport.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

static_assert(!std::is_copy_constructible_v<obz::transport::udp_socket>);
static_assert(std::is_nothrow_move_constructible_v<obz::transport::udp_socket>);
static_assert(std::is_nothrow_move_assignable_v<obz::transport::udp_socket>);

static_assert(!std::is_copy_constructible_v<obz::transport::tcp_socket>);
static_assert(std::is_nothrow_move_constructible_v<obz::transport::tcp_socket>);
static_assert(std::is_nothrow_move_assignable_v<obz::transport::tcp_socket>);

static_assert(!std::is_copy_constructible_v<obz::transport::tcp_listener>);
static_assert(std::is_nothrow_move_constructible_v<obz::transport::tcp_listener>);
static_assert(std::is_nothrow_move_assignable_v<obz::transport::tcp_listener>);

int main() {
    obz::blocking_queue<int> queue;
    queue.push(42);

    int value{};
    if (!queue.try_pop(value) || value != 42) {
        return 1;
    }

    std::array<std::byte, sizeof(std::uint32_t)> buffer{};
    obz::endian::write_be<std::uint32_t>(buffer, 0, 0x01020304);

    if (obz::endian::read_be<std::uint32_t>(buffer, 0) != 0x01020304) {
        return 2;
    }

    obz::transport::tcp_socket socket;
    if (socket.is_open()) {
        return 3;
    }

    obz::transport::udp_socket udp_socket;
    std::array<std::byte, 8> receive_buffer{};
    using udp_receive_type = decltype(udp_socket.receive_from(receive_buffer));
    static_assert(std::is_same_v<udp_receive_type, obz::transport::udp_receive_result>);

    if (udp_socket.is_open()) {
        return 4;
    }

    obz::spsc_ring_queue<int, 4> spsc;
    if (!spsc.try_push(17) || !spsc.try_pop(value) || value != 17) {
        return 5;
    }

    obz::mpsc_ring_queue<int, 4> mpsc;
    if (!mpsc.try_push(23) || !mpsc.try_pop(value) || value != 23) {
        return 6;
    }

    static_cast<void>(obz::thread_affinity_supported());
    return 0;
}
