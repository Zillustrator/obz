#include <obz/weighted_set.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

TEST_CASE("weighted_set starts empty") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.empty());
    REQUIRE(set.size() == 0);
    REQUIRE(set.total_weight() == 0);
}

TEST_CASE("weighted_set inserts unique values with positive weights") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("common", 10));
    REQUIRE(set.insert("rare", 1));

    REQUIRE_FALSE(set.empty());
    REQUIRE(set.size() == 2);
    REQUIRE(set.total_weight() == 11);
    REQUIRE(set.contains("common"));
    REQUIRE(set.contains("rare"));
    REQUIRE(set.weight_of("common") == 10);
    REQUIRE(set.weight_of("rare") == 1);
}

TEST_CASE("weighted_set rejects duplicate values") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("value", 3));
    REQUIRE_FALSE(set.insert("value", 7));

    REQUIRE(set.size() == 1);
    REQUIRE(set.total_weight() == 3);
    REQUIRE(set.weight_of("value") == 3);
}

TEST_CASE("weighted_set rejects zero weights") {
    obz::weighted_set<std::string> set;

    REQUIRE_THROWS_AS(set.insert("value", 0), std::invalid_argument);

    REQUIRE(set.insert("value", 1));
    REQUIRE_THROWS_AS(set.set_weight("value", 0), std::invalid_argument);
}

TEST_CASE("weighted_set erases values and updates total weight") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("a", 2));
    REQUIRE(set.insert("b", 5));
    REQUIRE(set.insert("c", 7));

    REQUIRE(set.erase("b"));
    REQUIRE_FALSE(set.contains("b"));
    REQUIRE(set.size() == 2);
    REQUIRE(set.total_weight() == 9);

    REQUIRE(set.contains("a"));
    REQUIRE(set.contains("c"));
    REQUIRE_FALSE(set.erase("missing"));
}

TEST_CASE("weighted_set updates existing weights") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("value", 4));

    REQUIRE(set.set_weight("value", 9));
    REQUIRE(set.weight_of("value") == 9);
    REQUIRE(set.total_weight() == 9);

    REQUIRE_FALSE(set.set_weight("missing", 3));
    REQUIRE(set.total_weight() == 9);
}

TEST_CASE("weighted_set throws when reading missing weight") {
    obz::weighted_set<std::string> set;

    REQUIRE_THROWS_AS(set.weight_of("missing"), std::out_of_range);
}

TEST_CASE("weighted_set chooses the only value") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("only", 42));

    std::mt19937 generator(123);

    for (int i = 0; i < 100; ++i) {
        REQUIRE(set.random(generator) == "only");
    }
}

TEST_CASE("weighted_set random returns contained values") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("common", 10));
    REQUIRE(set.insert("uncommon", 3));
    REQUIRE(set.insert("rare", 1));

    std::mt19937 generator(123);

    for (int i = 0; i < 100; ++i) {
        const auto& value = set.random(generator);
        REQUIRE(set.contains(value));
    }
}

TEST_CASE("weighted_set throws when choosing from empty set") {
    obz::weighted_set<std::string> set;
    std::mt19937 generator(123);

    REQUIRE_THROWS_AS(set.random(generator), std::runtime_error);
}

TEST_CASE("weighted_set rejects total weight overflow on insert") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("a", std::numeric_limits<std::uint64_t>::max()));
    REQUIRE_THROWS_AS(set.insert("b", 1), std::overflow_error);

    REQUIRE(set.size() == 1);
    REQUIRE(set.total_weight() == std::numeric_limits<std::uint64_t>::max());
    REQUIRE_FALSE(set.contains("b"));
}

TEST_CASE("weighted_set rejects total weight overflow on update") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("a", std::numeric_limits<std::uint64_t>::max() - 1));
    REQUIRE(set.insert("b", 1));

    REQUIRE_THROWS_AS(set.set_weight("b", 2), std::overflow_error);

    REQUIRE(set.weight_of("b") == 1);
    REQUIRE(set.total_weight() == std::numeric_limits<std::uint64_t>::max());
}

TEST_CASE("weighted_set clear removes all values") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("a", 2));
    REQUIRE(set.insert("b", 3));

    set.clear();

    REQUIRE(set.empty());
    REQUIRE(set.size() == 0);
    REQUIRE(set.total_weight() == 0);
    REQUIRE_FALSE(set.contains("a"));
    REQUIRE_FALSE(set.contains("b"));
}

TEST_CASE("weighted_set supports iteration over entries") {
    obz::weighted_set<std::string> set;

    REQUIRE(set.insert("a", 2));
    REQUIRE(set.insert("b", 3));

    std::uint64_t total = 0;
    std::vector<std::string> values;

    for (const auto& entry : set) {
        values.push_back(entry.first);
        total += entry.second;
    }

    REQUIRE(values.size() == 2);
    REQUIRE(total == set.total_weight());
}

TEST_CASE("weighted_set supports copy and move operations") {
    obz::weighted_set<std::string> original;

    REQUIRE(original.insert("a", 2));
    REQUIRE(original.insert("b", 3));

    obz::weighted_set<std::string> copied(original);

    REQUIRE(copied.size() == 2);
    REQUIRE(copied.total_weight() == 5);
    REQUIRE(copied.weight_of("a") == 2);
    REQUIRE(copied.weight_of("b") == 3);

    obz::weighted_set<std::string> copy_assigned;
    copy_assigned = copied;

    REQUIRE(copy_assigned.size() == 2);
    REQUIRE(copy_assigned.total_weight() == 5);
    REQUIRE(copy_assigned.contains("a"));
    REQUIRE(copy_assigned.contains("b"));

    obz::weighted_set<std::string> moved(std::move(copy_assigned));

    REQUIRE(moved.size() == 2);
    REQUIRE(moved.total_weight() == 5);
    REQUIRE(moved.weight_of("a") == 2);
    REQUIRE(moved.weight_of("b") == 3);

    obz::weighted_set<std::string> move_assigned;
    move_assigned = std::move(moved);

    REQUIRE(move_assigned.size() == 2);
    REQUIRE(move_assigned.total_weight() == 5);
    REQUIRE(move_assigned.contains("a"));
    REQUIRE(move_assigned.contains("b"));

    std::mt19937 generator(123);
    REQUIRE(move_assigned.contains(move_assigned.random(generator)));
}
