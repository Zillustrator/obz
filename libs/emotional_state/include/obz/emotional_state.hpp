#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace obz {

enum class mood_axis : int {
    negative_5 = -5,
    negative_4 = -4,
    negative_3 = -3,
    negative_2 = -2,
    negative_1 = -1,
    positive_1 = 1,
    positive_2 = 2,
    positive_3 = 3,
    positive_4 = 4,
    positive_5 = 5,
};

constexpr int value_of(mood_axis value) noexcept {
    return static_cast<int>(value);
}

namespace detail {

constexpr std::int16_t pack_mood_coordinates(mood_axis pleasantness, mood_axis energy) {
    return static_cast<std::int16_t>(
        ((value_of(pleasantness) + 5) << 4) | (value_of(energy) + 5));
}

} // namespace detail

enum class mood : std::int16_t {
    enraged = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::positive_5),
    panicked = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::positive_5),
    stressed = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::positive_5),
    jittery = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::positive_5),
    shocked = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::positive_5),
    surprised = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::positive_5),
    upbeat = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::positive_5),
    festive = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::positive_5),
    exhilarated = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::positive_5),
    ecstatic = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::positive_5),

    livid = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::positive_4),
    furious = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::positive_4),
    frustrated = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::positive_4),
    tense = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::positive_4),
    stunned = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::positive_4),
    hyper = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::positive_4),
    cheerful = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::positive_4),
    motivated = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::positive_4),
    inspired = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::positive_4),
    elated = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::positive_4),

    fuming = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::positive_3),
    frightened = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::positive_3),
    angry = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::positive_3),
    nervous = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::positive_3),
    restless = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::positive_3),
    energized = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::positive_3),
    lively = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::positive_3),
    excited = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::positive_3),
    optimistic = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::positive_3),
    enthusiastic = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::positive_3),

    anxious = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::positive_2),
    apprehensive = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::positive_2),
    worried = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::positive_2),
    irritated = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::positive_2),
    annoyed = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::positive_2),
    pleased = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::positive_2),
    focused = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::positive_2),
    happy = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::positive_2),
    proud = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::positive_2),
    thrilled = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::positive_2),

    repulsed = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::positive_1),
    troubled = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::positive_1),
    concerned = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::positive_1),
    uneasy = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::positive_1),
    peeved = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::positive_1),
    pleasant = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::positive_1),
    joyful = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::positive_1),
    hopeful = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::positive_1),
    playful = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::positive_1),
    blissful = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::positive_1),

    disgusted = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::negative_1),
    glum = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::negative_1),
    disappointed = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::negative_1),
    down = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::negative_1),
    apathetic = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::negative_1),
    at_ease = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::negative_1),
    easygoing = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::negative_1),
    content = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::negative_1),
    loving = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::negative_1),
    fulfilled = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::negative_1),

    pessimistic = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::negative_2),
    morose = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::negative_2),
    discouraged = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::negative_2),
    sad = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::negative_2),
    bored = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::negative_2),
    calm = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::negative_2),
    secure = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::negative_2),
    satisfied = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::negative_2),
    grateful = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::negative_2),
    touched = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::negative_2),

    alienated = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::negative_3),
    miserable = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::negative_3),
    lonely = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::negative_3),
    disheartened = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::negative_3),
    tired = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::negative_3),
    relaxed = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::negative_3),
    chill = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::negative_3),
    restful = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::negative_3),
    blessed = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::negative_3),
    balanced = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::negative_3),

    despondent = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::negative_4),
    depressed = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::negative_4),
    sullen = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::negative_4),
    exhausted = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::negative_4),
    fatigued = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::negative_4),
    mellow = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::negative_4),
    thoughtful = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::negative_4),
    peaceful = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::negative_4),
    comfortable = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::negative_4),
    carefree = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::negative_4),

    despairing = detail::pack_mood_coordinates(mood_axis::negative_5, mood_axis::negative_5),
    hopeless = detail::pack_mood_coordinates(mood_axis::negative_4, mood_axis::negative_5),
    desolate = detail::pack_mood_coordinates(mood_axis::negative_3, mood_axis::negative_5),
    spent = detail::pack_mood_coordinates(mood_axis::negative_2, mood_axis::negative_5),
    drained = detail::pack_mood_coordinates(mood_axis::negative_1, mood_axis::negative_5),
    sleepy = detail::pack_mood_coordinates(mood_axis::positive_1, mood_axis::negative_5),
    complacent = detail::pack_mood_coordinates(mood_axis::positive_2, mood_axis::negative_5),
    tranquil = detail::pack_mood_coordinates(mood_axis::positive_3, mood_axis::negative_5),
    cozy = detail::pack_mood_coordinates(mood_axis::positive_4, mood_axis::negative_5),
    serene = detail::pack_mood_coordinates(mood_axis::positive_5, mood_axis::negative_5),
};

enum class mood_quadrant {
    high_energy_low_pleasantness,
    high_energy_high_pleasantness,
    low_energy_low_pleasantness,
    low_energy_high_pleasantness,
};

inline constexpr double emotional_state_min_axis = -5.0;
inline constexpr double emotional_state_max_axis = 5.0;

namespace detail {

inline constexpr int mood_axis_min = value_of(mood_axis::negative_5);
inline constexpr int mood_axis_max = value_of(mood_axis::positive_5);

constexpr bool is_valid_mood_axis_value(int value) {
    return value >= mood_axis_min && value <= mood_axis_max && value != 0;
}

constexpr void validate_mood_axis(mood_axis value) {
    if (!is_valid_mood_axis_value(value_of(value))) {
        throw std::invalid_argument("mood axis value must be in -5..-1 or 1..5");
    }
}

} // namespace detail

constexpr mood_axis mood_axis_at(int value) {
    detail::validate_mood_axis(static_cast<mood_axis>(value));
    return static_cast<mood_axis>(value);
}

constexpr mood_axis pleasantness_of(mood value) {
    return mood_axis_at((static_cast<int>(value) >> 4) - 5);
}

constexpr mood_axis energy_of(mood value) {
    return mood_axis_at((static_cast<int>(value) & 0x0f) - 5);
}

constexpr mood mood_at(mood_axis pleasantness, mood_axis energy) {
    detail::validate_mood_axis(pleasantness);
    detail::validate_mood_axis(energy);

    return static_cast<mood>(detail::pack_mood_coordinates(pleasantness, energy));
}

constexpr mood_quadrant quadrant_at(mood_axis pleasantness, mood_axis energy) {
    detail::validate_mood_axis(pleasantness);
    detail::validate_mood_axis(energy);

    const auto pleasantness_value = value_of(pleasantness);
    const auto energy_value = value_of(energy);

    if (pleasantness_value < 0 && energy_value > 0) {
        return mood_quadrant::high_energy_low_pleasantness;
    }

    if (pleasantness_value > 0 && energy_value > 0) {
        return mood_quadrant::high_energy_high_pleasantness;
    }

    if (pleasantness_value < 0 && energy_value < 0) {
        return mood_quadrant::low_energy_low_pleasantness;
    }

    return mood_quadrant::low_energy_high_pleasantness;
}

constexpr mood_quadrant quadrant_of(mood value) {
    return quadrant_at(pleasantness_of(value), energy_of(value));
}

constexpr std::string_view name_of(mood value) {
    switch (value) {
    case mood::enraged: return "enraged";
    case mood::panicked: return "panicked";
    case mood::stressed: return "stressed";
    case mood::jittery: return "jittery";
    case mood::shocked: return "shocked";
    case mood::surprised: return "surprised";
    case mood::upbeat: return "upbeat";
    case mood::festive: return "festive";
    case mood::exhilarated: return "exhilarated";
    case mood::ecstatic: return "ecstatic";
    case mood::livid: return "livid";
    case mood::furious: return "furious";
    case mood::frustrated: return "frustrated";
    case mood::tense: return "tense";
    case mood::stunned: return "stunned";
    case mood::hyper: return "hyper";
    case mood::cheerful: return "cheerful";
    case mood::motivated: return "motivated";
    case mood::inspired: return "inspired";
    case mood::elated: return "elated";
    case mood::fuming: return "fuming";
    case mood::frightened: return "frightened";
    case mood::angry: return "angry";
    case mood::nervous: return "nervous";
    case mood::restless: return "restless";
    case mood::energized: return "energized";
    case mood::lively: return "lively";
    case mood::excited: return "excited";
    case mood::optimistic: return "optimistic";
    case mood::enthusiastic: return "enthusiastic";
    case mood::anxious: return "anxious";
    case mood::apprehensive: return "apprehensive";
    case mood::worried: return "worried";
    case mood::irritated: return "irritated";
    case mood::annoyed: return "annoyed";
    case mood::pleased: return "pleased";
    case mood::focused: return "focused";
    case mood::happy: return "happy";
    case mood::proud: return "proud";
    case mood::thrilled: return "thrilled";
    case mood::repulsed: return "repulsed";
    case mood::troubled: return "troubled";
    case mood::concerned: return "concerned";
    case mood::uneasy: return "uneasy";
    case mood::peeved: return "peeved";
    case mood::pleasant: return "pleasant";
    case mood::joyful: return "joyful";
    case mood::hopeful: return "hopeful";
    case mood::playful: return "playful";
    case mood::blissful: return "blissful";
    case mood::disgusted: return "disgusted";
    case mood::glum: return "glum";
    case mood::disappointed: return "disappointed";
    case mood::down: return "down";
    case mood::apathetic: return "apathetic";
    case mood::at_ease: return "at_ease";
    case mood::easygoing: return "easygoing";
    case mood::content: return "content";
    case mood::loving: return "loving";
    case mood::fulfilled: return "fulfilled";
    case mood::pessimistic: return "pessimistic";
    case mood::morose: return "morose";
    case mood::discouraged: return "discouraged";
    case mood::sad: return "sad";
    case mood::bored: return "bored";
    case mood::calm: return "calm";
    case mood::secure: return "secure";
    case mood::satisfied: return "satisfied";
    case mood::grateful: return "grateful";
    case mood::touched: return "touched";
    case mood::alienated: return "alienated";
    case mood::miserable: return "miserable";
    case mood::lonely: return "lonely";
    case mood::disheartened: return "disheartened";
    case mood::tired: return "tired";
    case mood::relaxed: return "relaxed";
    case mood::chill: return "chill";
    case mood::restful: return "restful";
    case mood::blessed: return "blessed";
    case mood::balanced: return "balanced";
    case mood::despondent: return "despondent";
    case mood::depressed: return "depressed";
    case mood::sullen: return "sullen";
    case mood::exhausted: return "exhausted";
    case mood::fatigued: return "fatigued";
    case mood::mellow: return "mellow";
    case mood::thoughtful: return "thoughtful";
    case mood::peaceful: return "peaceful";
    case mood::comfortable: return "comfortable";
    case mood::carefree: return "carefree";
    case mood::despairing: return "despairing";
    case mood::hopeless: return "hopeless";
    case mood::desolate: return "desolate";
    case mood::spent: return "spent";
    case mood::drained: return "drained";
    case mood::sleepy: return "sleepy";
    case mood::complacent: return "complacent";
    case mood::tranquil: return "tranquil";
    case mood::cozy: return "cozy";
    case mood::serene: return "serene";
    }

    throw std::invalid_argument("unknown mood value");
}

struct mood_change {
    mood previous;
    mood current;
};

struct quadrant_change {
    mood_quadrant previous;
    mood_quadrant current;
};

class emotional_state {
public:
    using mood_changed_callback = std::function<void(mood_change)>;
    using quadrant_changed_callback = std::function<void(quadrant_change)>;

    emotional_state(double pleasantness, double energy)
        : pleasantness_(clamped_coordinate(pleasantness)),
          energy_(clamped_coordinate(energy)),
          pleasantness_direction_(axis_direction(pleasantness_, 1)),
          energy_direction_(axis_direction(energy_, 1)) {
    }

    double pleasantness() const noexcept {
        return pleasantness_;
    }

    double energy() const noexcept {
        return energy_;
    }

    mood current_mood() const {
        return mood_at(
            snapped_axis_coordinate(pleasantness_, pleasantness_direction_),
            snapped_axis_coordinate(energy_, energy_direction_));
    }

    mood_quadrant current_quadrant() const {
        return quadrant_at(
            snapped_axis_coordinate(pleasantness_, pleasantness_direction_),
            snapped_axis_coordinate(energy_, energy_direction_));
    }

    void set_pleasantness(double value) {
        set_position(value, energy_);
    }

    void set_energy(double value) {
        set_position(pleasantness_, value);
    }

    void set_position(double pleasantness, double energy) {
        const auto next_pleasantness = clamped_coordinate(pleasantness);
        const auto next_energy = clamped_coordinate(energy);
        const auto previous_mood = current_mood();
        const auto previous_quadrant = current_quadrant();

        pleasantness_ = next_pleasantness;
        energy_ = next_energy;
        pleasantness_direction_ = axis_direction(pleasantness_, pleasantness_direction_);
        energy_direction_ = axis_direction(energy_, energy_direction_);

        const auto next_mood = current_mood();
        const auto next_quadrant = current_quadrant();

        if (next_mood != previous_mood && mood_changed_callback_) {
            mood_changed_callback_(mood_change{previous_mood, next_mood});
        }

        if (next_quadrant != previous_quadrant && quadrant_changed_callback_) {
            quadrant_changed_callback_(quadrant_change{previous_quadrant, next_quadrant});
        }
    }

    void adjust_pleasantness(double delta) {
        set_pleasantness(pleasantness_ + delta);
    }

    void adjust_energy(double delta) {
        set_energy(energy_ + delta);
    }

    void adjust(double pleasantness_delta, double energy_delta) {
        set_position(pleasantness_ + pleasantness_delta, energy_ + energy_delta);
    }

    void on_mood_changed(mood_changed_callback callback) {
        mood_changed_callback_ = std::move(callback);
    }

    void on_quadrant_changed(quadrant_changed_callback callback) {
        quadrant_changed_callback_ = std::move(callback);
    }

private:
    static double clamped_coordinate(double value) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("emotional_state coordinates must be finite");
        }

        return std::clamp(
            value,
            emotional_state_min_axis,
            emotional_state_max_axis);
    }

    static int axis_direction(double value, int previous_direction) noexcept {
        if (value < 0.0) {
            return -1;
        }

        if (value > 0.0) {
            return 1;
        }

        return previous_direction < 0 ? -1 : 1;
    }

    static mood_axis snapped_axis_coordinate(double value, int direction) noexcept {
        if (value == 0.0) {
            return direction < 0 ? mood_axis::negative_1 : mood_axis::positive_1;
        }

        auto snapped = value > 0.0
            ? static_cast<int>(value + 0.5)
            : static_cast<int>(value - 0.5);

        if (snapped == 0) {
            snapped = value < 0.0 ? -1 : 1;
        }

        return static_cast<mood_axis>(
            std::clamp(snapped, detail::mood_axis_min, detail::mood_axis_max));
    }

    double pleasantness_{1.0};
    double energy_{1.0};
    int pleasantness_direction_{1};
    int energy_direction_{1};
    mood_changed_callback mood_changed_callback_;
    quadrant_changed_callback quadrant_changed_callback_;
};

} // namespace obz
