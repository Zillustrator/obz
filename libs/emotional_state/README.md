# obz::emotional_state

A small emotional state model using continuous pleasantness and energy coordinates, with named moods mapped onto a finite grid.

The library has two layers. `mood` is the finite vocabulary layer: every named mood maps to one valid `(pleasantness, energy)` coordinate, and every valid grid coordinate maps back to one mood. `emotional_state` is the mutable layer: it stores continuous `double` coordinates, clamps them to `[-5.0, 5.0]`, and snaps them to the nearest finite mood when callers ask for the current mood or quadrant.

---

## Features

- 100 named mood states
- `enum class mood_axis` for finite non-zero grid coordinates
- `enum class mood` with a compact packed coordinate value
- Helpers for reading pleasantness and energy
- Lookup from grid coordinates to mood
- Quadrant classification
- Mutable `emotional_state` wrapper with continuous coordinates
- Clamped pleasantness and energy adjustment
- Mood-change and quadrant-change callbacks
- Header-only implementation

---

## Usage

```cpp
#include <obz/emotional_state.hpp>

obz::emotional_state state(1.0, -2.0);

state.adjust_pleasantness(2.0);

const auto current = state.current_mood();
const auto quadrant = state.current_quadrant();
```

---

## Basic Example

```cpp
#include <obz/emotional_state.hpp>

#include <iostream>

int main() {
    obz::emotional_state state(3.2, 1.8);

    std::cout << obz::name_of(state.current_mood()) << '\n';
    std::cout << state.pleasantness() << ", "
              << state.energy() << '\n';
}
```

---

## API

### mood

```cpp
enum class mood : std::int16_t;
```

Represents one of the 100 finite mood states.

The underlying value stores the grid coordinate in a compact form. This keeps the enum self-contained while making coordinate extraction cheap and deterministic.

---

### mood_axis

```cpp
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
```

Represents one finite non-zero grid coordinate on either axis.

Use `value_of(axis)` to read the integer value, and `mood_axis_at(value)` to validate and convert an integer to `mood_axis`.

---

### mood_at

```cpp
constexpr mood mood_at(mood_axis pleasantness, mood_axis energy);
```

Returns the mood at a valid grid coordinate.

Because both arguments are `mood_axis`, callers cannot pass zero or out-of-range integer coordinates directly. Invalid values created with an explicit enum cast are rejected with `std::invalid_argument`.

---

### pleasantness_of / energy_of

```cpp
constexpr mood_axis pleasantness_of(mood value);
constexpr mood_axis energy_of(mood value);
```

Return individual axes from a mood.

---

### quadrant_of

```cpp
constexpr mood_quadrant quadrant_of(mood value);
constexpr mood_quadrant quadrant_at(mood_axis pleasantness, mood_axis energy);
```

Classifies a mood or finite grid coordinate into one of four quadrants:

| Quadrant | Meaning |
|----------|---------|
| `high_energy_low_pleasantness` | red quadrant |
| `high_energy_high_pleasantness` | yellow quadrant |
| `low_energy_low_pleasantness` | blue quadrant |
| `low_energy_high_pleasantness` | green quadrant |

---

### name_of

```cpp
constexpr std::string_view name_of(mood value);
```

Returns a stable snake_case name for a mood.

---

### emotional_state

```cpp
class emotional_state {
public:
    emotional_state(double pleasantness, double energy);

    double pleasantness() const noexcept;
    double energy() const noexcept;

    mood current_mood() const;
    mood_quadrant current_quadrant() const;

    void set_pleasantness(double value);
    void set_energy(double value);
    void set_position(double pleasantness, double energy);

    void adjust_pleasantness(double delta);
    void adjust_energy(double delta);
    void adjust(double pleasantness_delta, double energy_delta);
};
```

Stores continuous pleasantness and energy coordinates as `double` values.

There is no default constructor. Callers must choose the initial emotional position explicitly because the finite mood grid has no neutral zero state.

Coordinates are clamped to:

```text
-5.0 <= coordinate <= 5.0
```

Non-finite values such as `NaN` and infinity throw `std::invalid_argument`.

---

### Callbacks

```cpp
struct mood_change {
    mood previous;
    mood current;
};

struct quadrant_change {
    mood_quadrant previous;
    mood_quadrant current;
};

void on_mood_changed(emotional_state::mood_changed_callback callback);
void on_quadrant_changed(emotional_state::quadrant_changed_callback callback);
```

Registers callbacks that run when the snapped mood or quadrant changes.

Passing an empty callback clears the handler:

```cpp
state.on_mood_changed({});
```

Callbacks run synchronously inside the setter or adjustment call that caused the change. The mood callback is checked and invoked before the quadrant callback.

If a callback throws, the exception propagates after the state has already been updated. Later callbacks for the same update are not run after an exception.

---

## Behaviour Summary

| Operation | Valid Input | Invalid Input |
|-----------|-------------|---------------|
| `mood_axis_at` | returns a mood axis | throws for zero or out-of-range integers |
| `mood_at` | returns a mood | throws for invalid casted mood axes |
| `quadrant_at` | returns a quadrant | throws for invalid casted mood axes |
| `quadrant_of` | returns a quadrant for a mood | n/a |
| `name_of` | returns text | throws for unknown enum values |
| `emotional_state` construction | stores clamped coordinates | throws for non-finite coordinates |
| `set_position` / adjustments | stores clamped coordinates and fires change callbacks | throws for non-finite coordinates |

---

## Design Notes

The pleasantness/energy grid is naturally expressed as `-5..-1` and `1..5` on both axes. The enum keeps that scale instead of normalising to `-1.0..1.0` or expanding to `-100..100`.

That choice makes the table easy to inspect:

```cpp
obz::mood::calm // x = 1, y = -2
```

It also leaves room for a later wrapper class to use `double` coordinates in the same `-5.0..5.0` space. The wrapper can clamp movement, cross quadrant boundaries, and round or snap back to the nearest finite mood without changing scales.

The enum values use a small biased packing scheme:

```text
packed = ((x + 5) << 4) | (y + 5)
```

Each axis fits into four bits after biasing by five. This is deliberately simple rather than clever: it makes conversion reversible and keeps the enum values tied to the grid.

The mutable wrapper stores continuous coordinates separately from the previous axis directions used for exact-zero snapping. This keeps two concerns separate:

- `pleasantness()` and `energy()` return the actual continuous state
- `current_mood()` and `current_quadrant()` return the nearest finite grid interpretation

The snapped finite coordinates are computed on demand and intentionally not exposed. Callers can read continuous coordinates from `emotional_state` or finite coordinates from a `mood` with `pleasantness_of` and `energy_of`, but they cannot mutate or depend on internal snapping state.

Because the finite grid has no zero coordinate, snapping uses one extra rule:

- non-zero values snap to the nearest non-zero integer coordinate
- small positive values snap to `1`
- small negative values snap to `-1`
- an exact `0.0` keeps the previous axis direction

If a state starts exactly on zero, the default previous direction is positive for that axis.

---

## Misuse Resistance

`mood_axis_at` rejects zero and out-of-range integers so callers cannot accidentally create a mood between grid cells.

`mood` and `mood_axis` are both `enum class` types, so they do not implicitly convert to integers. Callers must ask for integer axis values through `value_of`.

The quadrant API accepts finite grid coordinates as `mood_axis` values. The wrapper does not expose its internal zero-direction state.

`emotional_state` rejects `NaN` and infinity so the clamping rule cannot hide invalid numeric input.

Callbacks are optional and are registered explicitly. The wrapper stores one mood-change callback and one quadrant-change callback rather than a callback list, keeping this first version small and predictable.

---

## Implementation Notes

This first version models the vocabulary, coordinate mapping, and a small mutable wrapper.

The wrapper is not internally synchronized. Use it from one thread at a time, or protect shared access externally.

---

## When to Use

Use `emotional_state` when:

- code needs a finite mood vocabulary
- code needs a small mutable pleasantness/energy state
- a state machine needs an additional emotional dimension
- examples benefit from a small coordinate-to-enum mapping
- quadrant transitions are meaningful

---

## When Not to Use

Avoid `emotional_state` when:

- arbitrary emotion labels are needed
- continuous psychology modelling is required
- a simple boolean or small enum would communicate enough state
- callback ownership, threading, or event dispatch rules need a larger observer framework

---

## Potential Extensions

- Nearest-mood lookup from continuous coordinates
- Multiple callback subscribers
- Explicit callback clearing helpers
- Previous-mood and previous-quadrant accessors
- Compile-time table of all moods for iteration

---

## License

Part of the `obz` project.
