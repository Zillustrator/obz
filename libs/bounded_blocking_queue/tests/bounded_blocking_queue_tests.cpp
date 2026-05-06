#include <obz/bounded_blocking_queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

TEST_CASE("bounded_blocking_queue rejects zero capacity") {
    REQUIRE_THROWS_AS(obz::bounded_blocking_queue<int>(0), std::invalid_argument);
}

TEST_CASE("bounded_blocking_queue respects capacity") {
    obz::bounded_blocking_queue<int> queue(2);

    REQUIRE(queue.try_push(1));
    REQUIRE(queue.try_push(2));
    REQUIRE_FALSE(queue.try_push(3));

    REQUIRE(queue.full());
    REQUIRE(queue.size() == 2);
    REQUIRE(queue.capacity() == 2);
}

TEST_CASE("bounded_blocking_queue pops values in FIFO order") {
    obz::bounded_blocking_queue<int> queue(2);

    queue.push(1);
    queue.push(2);

    int value = 0;

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 1);

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 2);

    REQUIRE(queue.empty());
}

TEST_CASE("bounded_blocking_queue try_pop returns false when empty") {
    obz::bounded_blocking_queue<int> queue(2);

    int value = 0;

    REQUIRE_FALSE(queue.try_pop(value));
}

TEST_CASE("bounded_blocking_queue push blocks while queue is full") {
    obz::bounded_blocking_queue<int> queue(1);

    queue.push(1);

    std::atomic<bool> push_completed{false};

    std::thread producer([&] {
        queue.push(2);
        push_completed = true;
    });

    std::this_thread::sleep_for(50ms);

    REQUIRE_FALSE(push_completed.load());

    int value = 0;
    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 1);

    producer.join();

    REQUIRE(push_completed.load());

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 2);
}

TEST_CASE("bounded_blocking_queue wait_and_pop blocks while queue is empty") {
    obz::bounded_blocking_queue<int> queue(1);

    std::atomic<bool> pop_completed{false};
    int result = 0;

    std::thread consumer([&] {
        queue.wait_and_pop(result);
        pop_completed = true;
    });

    std::this_thread::sleep_for(50ms);

    REQUIRE_FALSE(pop_completed.load());

    queue.push(42);

    consumer.join();

    REQUIRE(pop_completed.load());
    REQUIRE(result == 42);
}

TEST_CASE("bounded_blocking_queue close wakes waiting consumer") {
    obz::bounded_blocking_queue<int> queue(1);

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

TEST_CASE("bounded_blocking_queue close wakes waiting producer") {
    obz::bounded_blocking_queue<int> queue(1);

    queue.push(1);

    std::atomic<bool> producer_finished{false};
    std::atomic<bool> producer_threw{false};

    std::thread producer([&] {
        try {
            queue.push(2);
        } catch (const std::runtime_error&) {
            producer_threw = true;
        }

        producer_finished = true;
    });

    std::this_thread::sleep_for(50ms);

    REQUIRE_FALSE(producer_finished.load());

    queue.close();

    producer.join();

    REQUIRE(producer_finished.load());
    REQUIRE(producer_threw.load());
}

TEST_CASE("bounded_blocking_queue throws when pushing after close") {
    obz::bounded_blocking_queue<int> queue(2);

    queue.close();

    REQUIRE_THROWS_AS(queue.push(1), std::runtime_error);
}

TEST_CASE("bounded_blocking_queue try_push returns false after close") {
    obz::bounded_blocking_queue<int> queue(2);

    queue.close();

    REQUIRE_FALSE(queue.try_push(1));
}

TEST_CASE("bounded_blocking_queue allows remaining values to be popped after close") {
    obz::bounded_blocking_queue<int> queue(2);

    queue.push(1);
    queue.push(2);
    queue.close();

    int value = 0;

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 1);

    REQUIRE(queue.wait_and_pop(value));
    REQUIRE(value == 2);

    REQUIRE_FALSE(queue.wait_and_pop(value));
    REQUIRE_FALSE(queue.try_pop(value));
}

TEST_CASE("bounded_blocking_queue closed reports shutdown state") {
    obz::bounded_blocking_queue<int> queue(2);

    REQUIRE_FALSE(queue.closed());

    queue.close();

    REQUIRE(queue.closed());

    queue.close();

    REQUIRE(queue.closed());
}

TEST_CASE("bounded_blocking_queue transfers values from multiple producers to multiple consumers") {
    constexpr int producer_count = 4;
    constexpr int consumer_count = 3;
    constexpr int values_per_producer = 10000;
    constexpr int total_count = producer_count * values_per_producer;

    obz::bounded_blocking_queue<int> queue(128);

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
