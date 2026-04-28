#include <obz/blocking_queue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <thread>

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

TEST_CASE("blocking_queue throws when pushing after close") {
    obz::blocking_queue<int> queue;

    queue.close();

    REQUIRE_THROWS(queue.push(1));
}