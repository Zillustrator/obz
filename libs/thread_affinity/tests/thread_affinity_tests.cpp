#include <obz/thread_affinity.hpp>

#include <catch2/catch_test_macros.hpp>

#include <limits>

TEST_CASE("thread_affinity support queries are noexcept") {
    STATIC_REQUIRE(noexcept(obz::thread_affinity_supported()));
    STATIC_REQUIRE(noexcept(obz::try_pin_current_thread_to_cpu(0)));

    static_cast<void>(obz::thread_affinity_supported());
}

TEST_CASE("thread_affinity try_pin_current_thread_to_cpu returns false for invalid CPU index") {
    constexpr auto invalid_cpu = std::numeric_limits<obz::cpu_index>::max();

    REQUIRE_FALSE(obz::try_pin_current_thread_to_cpu(invalid_cpu));
}

TEST_CASE("thread_affinity pin_current_thread_to_cpu throws for invalid CPU index") {
    constexpr auto invalid_cpu = std::numeric_limits<obz::cpu_index>::max();

    REQUIRE_THROWS(obz::pin_current_thread_to_cpu(invalid_cpu));
}
