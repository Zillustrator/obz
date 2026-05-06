#include <obz/ring_buffer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

TEST_CASE("ring_buffer rejects zero capacity") {
    REQUIRE_THROWS_AS(obz::ring_buffer<int>(0), std::invalid_argument);
}

TEST_CASE("ring_buffer starts empty") {
    obz::ring_buffer<int> buffer(3);

    REQUIRE(buffer.empty());
    REQUIRE_FALSE(buffer.full());
    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.capacity() == 3);
}

TEST_CASE("ring_buffer pushes and pops values in FIFO order") {
    obz::ring_buffer<int> buffer(3);

    REQUIRE(buffer.push_back(1));
    REQUIRE(buffer.push_back(2));
    REQUIRE(buffer.push_back(3));

    int value = 0;

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 1);

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 2);

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 3);

    REQUIRE(buffer.empty());
}

TEST_CASE("ring_buffer returns false when pushing to full buffer") {
    obz::ring_buffer<int> buffer(2);

    REQUIRE(buffer.push_back(1));
    REQUIRE(buffer.push_back(2));
    REQUIRE_FALSE(buffer.push_back(3));

    REQUIRE(buffer.full());
    REQUIRE(buffer.size() == 2);
}

TEST_CASE("ring_buffer returns false when popping from empty buffer") {
    obz::ring_buffer<int> buffer(2);

    int value = 0;

    REQUIRE_FALSE(buffer.pop_front(value));
}

TEST_CASE("ring_buffer wraps around correctly") {
    obz::ring_buffer<int> buffer(3);

    REQUIRE(buffer.push_back(1));
    REQUIRE(buffer.push_back(2));
    REQUIRE(buffer.push_back(3));

    int value = 0;

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 1);

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 2);

    REQUIRE(buffer.push_back(4));
    REQUIRE(buffer.push_back(5));

    REQUIRE(buffer.full());

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 3);

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 4);

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 5);

    REQUIRE(buffer.empty());
}

TEST_CASE("ring_buffer front returns next value without removing it") {
    obz::ring_buffer<int> buffer(2);

    REQUIRE(buffer.push_back(10));
    REQUIRE(buffer.push_back(20));

    REQUIRE(buffer.front() == 10);
    REQUIRE(buffer.size() == 2);

    int value = 0;
    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 10);

    REQUIRE(buffer.front() == 20);
}

TEST_CASE("ring_buffer front returns const reference for const buffer") {
    obz::ring_buffer<int> buffer(2);

    REQUIRE(buffer.push_back(10));

    const auto& const_buffer = buffer;

    REQUIRE(const_buffer.front() == 10);
    REQUIRE(const_buffer.size() == 1);
}

TEST_CASE("ring_buffer throws when accessing front of empty buffer") {
    obz::ring_buffer<int> buffer(2);

    REQUIRE_THROWS_AS(buffer.front(), std::runtime_error);
}

TEST_CASE("ring_buffer clear removes all values") {
    obz::ring_buffer<int> buffer(3);

    REQUIRE(buffer.push_back(1));
    REQUIRE(buffer.push_back(2));

    buffer.clear();

    REQUIRE(buffer.empty());
    REQUIRE(buffer.size() == 0);
    REQUIRE_FALSE(buffer.full());

    int value = 0;
    REQUIRE_FALSE(buffer.pop_front(value));

    REQUIRE(buffer.push_back(3));
    REQUIRE(buffer.pop_front(value));
    REQUIRE(value == 3);
    REQUIRE(buffer.empty());
}

TEST_CASE("ring_buffer supports non-default-constructible types") {
    struct non_default_constructible {
        explicit non_default_constructible(int value)
            : value(value) {}

        int value;
    };

    obz::ring_buffer<non_default_constructible> buffer(2);

    REQUIRE(buffer.emplace_back(10));
    REQUIRE(buffer.emplace_back(20));
    REQUIRE_FALSE(buffer.emplace_back(30));

    non_default_constructible value(0);

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value.value == 10);

    REQUIRE(buffer.pop_front(value));
    REQUIRE(value.value == 20);

    REQUIRE(buffer.empty());
}

TEST_CASE("ring_buffer destroys remaining values on destruction") {
    struct tracked {
        explicit tracked(int value, int& destroyed_count)
            : value(value),
              destroyed_count(&destroyed_count) {}

        tracked(const tracked&) = delete;
        tracked& operator=(const tracked&) = delete;

        tracked(tracked&& other) noexcept
            : value(other.value),
              destroyed_count(other.destroyed_count) {
            other.destroyed_count = nullptr;
        }

        tracked& operator=(tracked&& other) noexcept {
            value = other.value;
            destroyed_count = other.destroyed_count;
            other.destroyed_count = nullptr;
            return *this;
        }

        ~tracked() {
            if (destroyed_count != nullptr) {
                ++(*destroyed_count);
            }
        }

        int value;
        int* destroyed_count;
    };

    int destroyed_count = 0;

    {
        obz::ring_buffer<tracked> buffer(3);

        REQUIRE(buffer.emplace_back(1, destroyed_count));
        REQUIRE(buffer.emplace_back(2, destroyed_count));
    }

    REQUIRE(destroyed_count == 2);
}
