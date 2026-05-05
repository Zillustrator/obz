#include <obz/mpsc_ring_queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <thread>
#include <vector>

TEST_CASE("mpsc_ring_queue starts empty") {
    obz::mpsc_ring_queue<int, 4> queue;

    REQUIRE(queue.empty());
    REQUIRE_FALSE(queue.full());
    REQUIRE(queue.size() == 0);
    REQUIRE(queue.capacity() == 4);
}

TEST_CASE("mpsc_ring_queue pushes and pops values in FIFO order on one thread") {
    obz::mpsc_ring_queue<int, 4> queue;

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

TEST_CASE("mpsc_ring_queue uses full declared capacity") {
    obz::mpsc_ring_queue<int, 3> queue;

    REQUIRE(queue.try_push(1));
    REQUIRE(queue.try_push(2));
    REQUIRE(queue.try_push(3));

    REQUIRE(queue.full());
    REQUIRE(queue.size() == 3);

    REQUIRE_FALSE(queue.try_push(4));
}

TEST_CASE("mpsc_ring_queue returns false when popping from empty queue") {
    obz::mpsc_ring_queue<int, 2> queue;

    int value = 0;

    REQUIRE_FALSE(queue.try_pop(value));
}

TEST_CASE("mpsc_ring_queue wraps around correctly") {
    obz::mpsc_ring_queue<int, 3> queue;

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

TEST_CASE("mpsc_ring_queue supports non-default-constructible types") {
    struct non_default_constructible {
        explicit non_default_constructible(int value) noexcept
            : value(value) {}

        non_default_constructible& operator=(non_default_constructible&& other) noexcept {
            value = other.value;
            return *this;
        }

        int value;
    };

    obz::mpsc_ring_queue<non_default_constructible, 2> queue;

    REQUIRE(queue.try_emplace(10));
    REQUIRE(queue.try_emplace(20));
    REQUIRE_FALSE(queue.try_emplace(30));

    non_default_constructible value(0);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value.value == 10);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value.value == 20);
}

TEST_CASE("mpsc_ring_queue clear removes all published values") {
    obz::mpsc_ring_queue<int, 3> queue;

    REQUIRE(queue.try_push(1));
    REQUIRE(queue.try_push(2));

    queue.clear();

    REQUIRE(queue.empty());
    REQUIRE(queue.size() == 0);

    int value = 0;
    REQUIRE_FALSE(queue.try_pop(value));
}

TEST_CASE("mpsc_ring_queue transfers values from multiple producers to one consumer") {
    constexpr int producer_count = 4;
    constexpr int values_per_producer = 25000;
    constexpr int total_count = producer_count * values_per_producer;

    obz::mpsc_ring_queue<int, 1024> queue;

    std::atomic<int> producers_done{0};
    std::vector<std::thread> producers;
    producers.reserve(producer_count);

    for (int producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([&, producer] {
            const int base = producer * values_per_producer;

            for (int offset = 0; offset < values_per_producer; ++offset) {
                while (!queue.try_push(base + offset)) {
                    std::this_thread::yield();
                }
            }

            producers_done.fetch_add(1, std::memory_order_release);
        });
    }

    std::vector<int> received;
    received.reserve(total_count);

    std::thread consumer([&] {
        int value = 0;

        while (producers_done.load(std::memory_order_acquire) != producer_count ||
               !queue.empty()) {
            if (queue.try_pop(value)) {
                received.push_back(value);
            } else {
                std::this_thread::yield();
            }
        }
    });

    for (auto& producer : producers) {
        producer.join();
    }

    consumer.join();

    REQUIRE(received.size() == total_count);

    std::vector<bool> seen(total_count, false);

    for (const int value : received) {
        REQUIRE(value >= 0);
        REQUIRE(value < total_count);
        REQUIRE_FALSE(seen[value]);
        seen[value] = true;
    }

    for (const bool was_seen : seen) {
        REQUIRE(was_seen);
    }

    REQUIRE(queue.empty());
}
