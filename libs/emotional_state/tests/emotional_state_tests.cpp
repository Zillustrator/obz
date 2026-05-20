#include <obz/emotional_state.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

constexpr auto all_moods = std::array{
    obz::mood::enraged,
    obz::mood::panicked,
    obz::mood::stressed,
    obz::mood::jittery,
    obz::mood::shocked,
    obz::mood::surprised,
    obz::mood::upbeat,
    obz::mood::festive,
    obz::mood::exhilarated,
    obz::mood::ecstatic,
    obz::mood::livid,
    obz::mood::furious,
    obz::mood::frustrated,
    obz::mood::tense,
    obz::mood::stunned,
    obz::mood::hyper,
    obz::mood::cheerful,
    obz::mood::motivated,
    obz::mood::inspired,
    obz::mood::elated,
    obz::mood::fuming,
    obz::mood::frightened,
    obz::mood::angry,
    obz::mood::nervous,
    obz::mood::restless,
    obz::mood::energized,
    obz::mood::lively,
    obz::mood::excited,
    obz::mood::optimistic,
    obz::mood::enthusiastic,
    obz::mood::anxious,
    obz::mood::apprehensive,
    obz::mood::worried,
    obz::mood::irritated,
    obz::mood::annoyed,
    obz::mood::pleased,
    obz::mood::focused,
    obz::mood::happy,
    obz::mood::proud,
    obz::mood::thrilled,
    obz::mood::repulsed,
    obz::mood::troubled,
    obz::mood::concerned,
    obz::mood::uneasy,
    obz::mood::peeved,
    obz::mood::pleasant,
    obz::mood::joyful,
    obz::mood::hopeful,
    obz::mood::playful,
    obz::mood::blissful,
    obz::mood::disgusted,
    obz::mood::glum,
    obz::mood::disappointed,
    obz::mood::down,
    obz::mood::apathetic,
    obz::mood::at_ease,
    obz::mood::easygoing,
    obz::mood::content,
    obz::mood::loving,
    obz::mood::fulfilled,
    obz::mood::pessimistic,
    obz::mood::morose,
    obz::mood::discouraged,
    obz::mood::sad,
    obz::mood::bored,
    obz::mood::calm,
    obz::mood::secure,
    obz::mood::satisfied,
    obz::mood::grateful,
    obz::mood::touched,
    obz::mood::alienated,
    obz::mood::miserable,
    obz::mood::lonely,
    obz::mood::disheartened,
    obz::mood::tired,
    obz::mood::relaxed,
    obz::mood::chill,
    obz::mood::restful,
    obz::mood::blessed,
    obz::mood::balanced,
    obz::mood::despondent,
    obz::mood::depressed,
    obz::mood::sullen,
    obz::mood::exhausted,
    obz::mood::fatigued,
    obz::mood::mellow,
    obz::mood::thoughtful,
    obz::mood::peaceful,
    obz::mood::comfortable,
    obz::mood::carefree,
    obz::mood::despairing,
    obz::mood::hopeless,
    obz::mood::desolate,
    obz::mood::spent,
    obz::mood::drained,
    obz::mood::sleepy,
    obz::mood::complacent,
    obz::mood::tranquil,
    obz::mood::cozy,
    obz::mood::serene,
};

} // namespace

TEST_CASE("mood stores pleasantness and energy in the enum value") {
    REQUIRE(obz::pleasantness_of(obz::mood::enraged) == obz::mood_axis::negative_5);
    REQUIRE(obz::energy_of(obz::mood::enraged) == obz::mood_axis::positive_5);
    REQUIRE(obz::pleasantness_of(obz::mood::ecstatic) == obz::mood_axis::positive_5);
    REQUIRE(obz::energy_of(obz::mood::ecstatic) == obz::mood_axis::positive_5);
    REQUIRE(obz::pleasantness_of(obz::mood::at_ease) == obz::mood_axis::positive_1);
    REQUIRE(obz::energy_of(obz::mood::at_ease) == obz::mood_axis::negative_1);
    REQUIRE(obz::pleasantness_of(obz::mood::serene) == obz::mood_axis::positive_5);
    REQUIRE(obz::energy_of(obz::mood::serene) == obz::mood_axis::negative_5);
}

TEST_CASE("mood can be looked up from valid grid coordinates") {
    REQUIRE(obz::mood_at(obz::mood_axis::negative_5, obz::mood_axis::positive_5)
        == obz::mood::enraged);
    REQUIRE(obz::mood_at(obz::mood_axis::positive_3, obz::mood_axis::positive_2)
        == obz::mood::happy);
    REQUIRE(obz::mood_at(obz::mood_axis::positive_1, obz::mood_axis::negative_2)
        == obz::mood::calm);
    REQUIRE(obz::mood_at(obz::mood_axis::positive_5, obz::mood_axis::negative_5)
        == obz::mood::serene);
}

TEST_CASE("all moods round trip through finite axes") {
    for (const auto value : all_moods) {
        const auto pleasantness = obz::pleasantness_of(value);
        const auto energy = obz::energy_of(value);

        REQUIRE(obz::mood_at(pleasantness, energy) == value);
        REQUIRE(obz::quadrant_of(value) == obz::quadrant_at(pleasantness, energy));
        REQUIRE_FALSE(obz::name_of(value).empty());
    }
}

TEST_CASE("mood_axis rejects values outside the finite grid") {
    REQUIRE(obz::value_of(obz::mood_axis::negative_5) == -5);
    REQUIRE(obz::value_of(obz::mood_axis::positive_5) == 5);

    REQUIRE(obz::mood_axis_at(-5) == obz::mood_axis::negative_5);
    REQUIRE(obz::mood_axis_at(5) == obz::mood_axis::positive_5);

    REQUIRE_THROWS_AS(obz::mood_axis_at(0), std::invalid_argument);
    REQUIRE_THROWS_AS(obz::mood_axis_at(-6), std::invalid_argument);
    REQUIRE_THROWS_AS(obz::mood_axis_at(6), std::invalid_argument);
}

TEST_CASE("mood lookup rejects invalid casted mood axes") {
    const auto zero_axis = static_cast<obz::mood_axis>(0);
    const auto out_of_range_axis = static_cast<obz::mood_axis>(6);

    REQUIRE_THROWS_AS(
        obz::mood_at(zero_axis, obz::mood_axis::positive_1),
        std::invalid_argument);
    REQUIRE_THROWS_AS(
        obz::mood_at(obz::mood_axis::positive_1, out_of_range_axis),
        std::invalid_argument);
    REQUIRE_THROWS_AS(
        obz::quadrant_at(zero_axis, obz::mood_axis::positive_1),
        std::invalid_argument);
    REQUIRE_THROWS_AS(
        obz::quadrant_at(obz::mood_axis::positive_1, out_of_range_axis),
        std::invalid_argument);
}

TEST_CASE("mood quadrants follow pleasantness and energy signs") {
    REQUIRE(obz::quadrant_at(obz::mood_axis::negative_1, obz::mood_axis::positive_1)
        == obz::mood_quadrant::high_energy_low_pleasantness);
    REQUIRE(obz::quadrant_at(obz::mood_axis::positive_1, obz::mood_axis::positive_1)
        == obz::mood_quadrant::high_energy_high_pleasantness);
    REQUIRE(obz::quadrant_at(obz::mood_axis::negative_1, obz::mood_axis::negative_1)
        == obz::mood_quadrant::low_energy_low_pleasantness);
    REQUIRE(obz::quadrant_at(obz::mood_axis::positive_1, obz::mood_axis::negative_1)
        == obz::mood_quadrant::low_energy_high_pleasantness);
    REQUIRE(obz::quadrant_of(obz::mood::enraged)
        == obz::mood_quadrant::high_energy_low_pleasantness);
    REQUIRE(obz::quadrant_of(obz::mood::ecstatic)
        == obz::mood_quadrant::high_energy_high_pleasantness);
    REQUIRE(obz::quadrant_of(obz::mood::despairing)
        == obz::mood_quadrant::low_energy_low_pleasantness);
    REQUIRE(obz::quadrant_of(obz::mood::serene)
        == obz::mood_quadrant::low_energy_high_pleasantness);
}

TEST_CASE("mood names are available as stable snake_case text") {
    REQUIRE(obz::name_of(obz::mood::at_ease).compare("at_ease") == 0);
    REQUIRE(obz::name_of(obz::mood::ecstatic).compare("ecstatic") == 0);
}

TEST_CASE("emotional_state starts from explicit continuous coordinates") {
    const obz::emotional_state state(1.0, 1.0);

    REQUIRE(state.pleasantness() == 1.0);
    REQUIRE(state.energy() == 1.0);
    REQUIRE(state.current_mood() == obz::mood::pleasant);
    REQUIRE(state.current_quadrant()
        == obz::mood_quadrant::high_energy_high_pleasantness);
}

TEST_CASE("emotional_state clamps continuous coordinates") {
    const obz::emotional_state state(9.0, -8.0);

    REQUIRE(state.pleasantness() == 5.0);
    REQUIRE(state.energy() == -5.0);
    REQUIRE(state.current_mood() == obz::mood::serene);
}

TEST_CASE("emotional_state rejects non-finite coordinates") {
    REQUIRE_THROWS_AS(obz::emotional_state(0.0, std::numeric_limits<double>::infinity()),
        std::invalid_argument);

    obz::emotional_state state(1.0, 1.0);

    REQUIRE_THROWS_AS(state.set_pleasantness(std::numeric_limits<double>::quiet_NaN()),
        std::invalid_argument);
}

TEST_CASE("emotional_state snaps small non-zero values to the nearest non-zero mood coordinate") {
    obz::emotional_state state(0.2, -0.2);

    REQUIRE(state.pleasantness() == 0.2);
    REQUIRE(state.energy() == -0.2);
    REQUIRE(state.current_mood() == obz::mood::at_ease);

    state.set_position(-0.2, 0.2);

    REQUIRE(state.current_mood() == obz::mood::peeved);
}

TEST_CASE("emotional_state favours previous axis direction when continuous coordinates land on zero") {
    obz::emotional_state state(-2.0, 3.0);

    REQUIRE(state.current_mood() == obz::mood::nervous);

    state.set_position(0.0, 0.0);

    REQUIRE(state.pleasantness() == 0.0);
    REQUIRE(state.energy() == 0.0);
    REQUIRE(state.current_mood() == obz::mood::peeved);
}

TEST_CASE("emotional_state can update without registered callbacks") {
    obz::emotional_state state(1.0, 1.0);

    REQUIRE_NOTHROW(state.set_position(-1.0, -1.0));
    REQUIRE(state.current_mood() == obz::mood::apathetic);
}

TEST_CASE("emotional_state adjusts pleasantness and energy") {
    obz::emotional_state state(1.0, 1.0);

    state.adjust_pleasantness(2.2);
    REQUIRE(state.pleasantness() == 3.2);
    REQUIRE(state.current_mood() == obz::mood::hopeful);

    state.adjust_energy(-3.1);
    REQUIRE(state.energy() == -2.1);
    REQUIRE(state.current_mood() == obz::mood::satisfied);

    state.adjust(10.0, -10.0);
    REQUIRE(state.pleasantness() == 5.0);
    REQUIRE(state.energy() == -5.0);
    REQUIRE(state.current_mood() == obz::mood::serene);
}

TEST_CASE("emotional_state invokes callbacks when mood or quadrant changes") {
    obz::emotional_state state(1.0, 1.0);
    std::vector<obz::mood_change> mood_changes;
    std::vector<obz::quadrant_change> quadrant_changes;

    state.on_mood_changed([&](obz::mood_change change) {
        mood_changes.push_back(change);
    });
    state.on_quadrant_changed([&](obz::quadrant_change change) {
        quadrant_changes.push_back(change);
    });

    state.set_position(2.0, 1.0);

    REQUIRE(mood_changes.size() == 1);
    REQUIRE(mood_changes[0].previous == obz::mood::pleasant);
    REQUIRE(mood_changes[0].current == obz::mood::joyful);
    REQUIRE(quadrant_changes.empty());

    state.set_position(-2.0, 1.0);

    REQUIRE(mood_changes.size() == 2);
    REQUIRE(mood_changes[1].previous == obz::mood::joyful);
    REQUIRE(mood_changes[1].current == obz::mood::uneasy);
    REQUIRE(quadrant_changes.size() == 1);
    REQUIRE(quadrant_changes[0].previous
        == obz::mood_quadrant::high_energy_high_pleasantness);
    REQUIRE(quadrant_changes[0].current
        == obz::mood_quadrant::high_energy_low_pleasantness);
}

TEST_CASE("emotional_state stops later callbacks when an earlier callback throws") {
    obz::emotional_state state(1.0, 1.0);
    bool quadrant_callback_was_called{false};

    state.on_mood_changed([](obz::mood_change) {
        throw std::runtime_error("mood callback failed");
    });
    state.on_quadrant_changed([&](obz::quadrant_change) {
        quadrant_callback_was_called = true;
    });

    REQUIRE_THROWS_AS(state.set_position(-1.0, -1.0), std::runtime_error);
    REQUIRE(state.current_mood() == obz::mood::apathetic);
    REQUIRE(state.current_quadrant()
        == obz::mood_quadrant::low_energy_low_pleasantness);
    REQUIRE_FALSE(quadrant_callback_was_called);
}
