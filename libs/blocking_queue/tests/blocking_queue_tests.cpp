#include <obz/blocking_queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

TEST_CASE("blocking_queue stores and retrieves values in FIFO order") {
    obz::blocking_queue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.push(3);

    int value = 0;

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 1);

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 2);

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 3);

    REQUIRE(queue.empty());
}

TEST_CASE("blocking_queue try_pop returns false when empty") {
    obz::blocking_queue<int> queue;

    int value = 0;

    REQUIRE_FALSE(queue.try_pop(value));
}

TEST_CASE("blocking_queue wakes waiting thread when value is pushed") {
    obz::blocking_queue<int> queue;

    int result = 0;

    std::thread worker([&] {
        queue.wait_and_pop(result);
    });

    queue.push(42);

    worker.join();

    REQUIRE(result == 42);
}

TEST_CASE("blocking_queue wait_and_pop returns false after close") {
    obz::blocking_queue<int> queue;

    queue.close();

    int value = 0;

    REQUIRE_FALSE(queue.wait_and_pop(value));
}

TEST_CASE("blocking_queue close wakes waiting consumer") {
    obz::blocking_queue<int> queue;

    std::atomic<bool> consumer_finished{false};
    bool pop_result = true;

    std::thread consumer([&] {
        int value = 0;
        pop_result = queue.wait_and_pop(value);
        consumer_finished = true;
    });

    std::this_thread::sleep_for(50ms);

    REQUIRE_FALSE(consumer_finished.load());

    queue.close();

    consumer.join();

    REQUIRE(consumer_finished.load());
    REQUIRE_FALSE(pop_result);
}

TEST_CASE("blocking_queue allows remaining values to be popped after close") {
    obz::blocking_queue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.close();

    int value = 0;

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 1);

    REQUIRE(queue.try_pop(value));
    REQUIRE(value == 2);

    REQUIRE_FALSE(queue.wait_and_pop(value));
    REQUIRE_FALSE(queue.try_pop(value));
}

TEST_CASE("blocking_queue closed reports shutdown state") {
    obz::blocking_queue<int> queue;

    REQUIRE_FALSE(queue.closed());

    queue.close();

    REQUIRE(queue.closed());

    queue.close();

    REQUIRE(queue.closed());
}

TEST_CASE("blocking_queue throws when pushing after close") {
    obz::blocking_queue<int> queue;

    queue.close();

    REQUIRE_THROWS_AS(queue.push(1), std::runtime_error);
}

TEST_CASE("blocking_queue transfers values from multiple producers to multiple consumers") {
    constexpr int producer_count = 4;
    constexpr int consumer_count = 3;
    constexpr int values_per_producer = 10000;
    constexpr int total_count = producer_count * values_per_producer;

    obz::blocking_queue<int> queue;

    std::vector<int> received;
    received.reserve(total_count);

    std::mutex received_mutex;

    std::vector<std::thread> consumers;
    consumers.reserve(consumer_count);

    for (int consumer = 0; consumer < consumer_count; ++consumer) {
        consumers.emplace_back([&] {
            int value = 0;

            while (queue.wait_and_pop(value)) {
                std::lock_guard lock(received_mutex);
                received.push_back(value);
            }
        });
    }

    std::vector<std::thread> producers;
    producers.reserve(producer_count);

    for (int producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([&, producer] {
            const int base = producer * values_per_producer;

            for (int offset = 0; offset < values_per_producer; ++offset) {
                queue.push(base + offset);
            }
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }

    queue.close();

    for (auto& consumer : consumers) {
        consumer.join();
    }

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
}
