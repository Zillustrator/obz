#include <obz/spsc_ring_queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <thread>
#include <vector>

TEST_CASE("spsc_ring_queue starts empty") {
    obz::spsc_ring_queue<int, 4> queue;

    REQUIRE(queue.empty());
    REQUIRE_FALSE(queue.full());
    REQUIRE(queue.size() == 0);
    REQUIRE(queue.capacity() == 4);
}

TEST_CASE("spsc_ring_queue pushes and pops values in FIFO order") {
    obz::spsc_ring_queue<int, 4> queue;

    REQUIRE(queue.try_push(1));
    REQUIRE(queue.try_push(2));
    REQUIRE(queue.try_push(3));

    int value = 0;

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 1);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 2);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 3);

    REQUIRE(queue.empty());
}

TEST_CASE("spsc_ring_queue uses full declared capacity") {
    obz::spsc_ring_queue<int, 3> queue;

    REQUIRE(queue.try_push(1));
    REQUIRE(queue.try_push(2));
    REQUIRE(queue.try_push(3));

    REQUIRE(queue.full());
    REQUIRE(queue.size() == 3);

    REQUIRE_FALSE(queue.try_push(4));
}

TEST_CASE("spsc_ring_queue returns false when popping from empty queue") {
    obz::spsc_ring_queue<int, 2> queue;

    int value = 0;

    REQUIRE_FALSE(queue.try_pop(value));
}

TEST_CASE("spsc_ring_queue wraps around correctly") {
    obz::spsc_ring_queue<int, 3> queue;

    REQUIRE(queue.try_push(1));
    REQUIRE(queue.try_push(2));
    REQUIRE(queue.try_push(3));

    int value = 0;

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 1);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 2);

    REQUIRE(queue.try_push(4));
    REQUIRE(queue.try_push(5));

    REQUIRE(queue.full());

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 3);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 4);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 5);

    REQUIRE(queue.empty());
}

TEST_CASE("spsc_ring_queue supports non-default-constructible types") {
    struct non_default_constructible {
        explicit non_default_constructible(int value)
            : value(value) {}

        int value;
    };

    obz::spsc_ring_queue<non_default_constructible, 2> queue;

    REQUIRE(queue.try_emplace(10));
    REQUIRE(queue.try_emplace(20));
    REQUIRE_FALSE(queue.try_emplace(30));

    non_default_constructible value(0);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value.value == 10);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value.value == 20);
}

TEST_CASE("spsc_ring_queue clear removes all values") {
    obz::spsc_ring_queue<int, 3> queue;

    REQUIRE(queue.try_push(1));
    REQUIRE(queue.try_push(2));

    queue.clear();

    REQUIRE(queue.empty());
    REQUIRE(queue.size() == 0);

    int value = 0;
    REQUIRE_FALSE(queue.try_pop(value));
}

TEST_CASE("spsc_ring_queue transfers values between one producer and one consumer") {
    constexpr int count = 100000;

    obz::spsc_ring_queue<int, 1024> queue;

    std::atomic<bool> producer_done{false};

    std::thread producer([&] {
        for (int i = 0; i < count; ++i) {
            while (!queue.try_push(i)) {
                std::this_thread::yield();
            }
        }

        producer_done = true;
    });

    std::vector<int> received;
    received.reserve(count);

    std::thread consumer([&] {
        int value = 0;

        while (!producer_done.load() || !queue.empty()) {
            if (queue.try_pop(value)) {
                received.push_back(value);
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    REQUIRE(received.size() == count);

    for (int i = 0; i < count; ++i) {
        REQUIRE(received[i] == i);
    }

    REQUIRE(queue.empty());
}