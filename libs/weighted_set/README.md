# obz::weighted_set

A collection of unique values where each value has a positive integer weight used for random selection.

`weighted_set` solves the common problem of choosing among unique items with different probabilities while keeping the API small and deterministic to test. The class owns the invariant that every stored value is unique, every weight is positive, and `total_weight()` is the sum of all stored weights.

---

## Features

- Unique values
- Positive `std::uint64_t` weights
- Random weighted selection
- Caller-supplied random engine
- O(1) average lookup, update, and erase by value
- Deterministic testing through seeded random engines
- Header-only implementation

---

## Usage

```cpp
#include <obz/weighted_set.hpp>

#include <random>
#include <string>

obz::weighted_set<std::string> set;

set.insert("common", 10);
set.insert("rare", 1);

std::mt19937 generator(123);
const std::string& value = set.random(generator);
```

---

## Basic Example

```cpp
#include <obz/weighted_set.hpp>

#include <iostream>
#include <random>
#include <string>

int main() {
    obz::weighted_set<std::string> loot;

    loot.insert("coin", 80);
    loot.insert("gem", 15);
    loot.insert("relic", 5);

    std::mt19937 generator(std::random_device{}());

    std::cout << loot.random(generator) << '\n';
}
```

---

## API

### insert

```cpp
bool insert(T value, weight_type weight);
```

Adds a new value with a positive weight.

Returns:
- `true` if the value was inserted
- `false` if the value already exists

Throws:
- `std::invalid_argument` if `weight == 0`
- `std::overflow_error` if adding the weight would overflow `total_weight()`

---

### erase

```cpp
bool erase(const T& value);
```

Removes a value and subtracts its weight from the total.

Returns:
- `true` if the value was removed
- `false` if the value was not present

---

### set_weight

```cpp
bool set_weight(const T& value, weight_type weight);
```

Changes the weight for an existing value.

Returns:
- `true` if the value exists and the weight was updated
- `false` if the value was not present

Throws:
- `std::invalid_argument` if `weight == 0`
- `std::overflow_error` if the new weight would overflow `total_weight()`

---

### weight_of

```cpp
weight_type weight_of(const T& value) const;
```

Returns the weight associated with `value`.

Throws `std::out_of_range` if the value is not present.

---

### contains

```cpp
bool contains(const T& value) const;
```

Returns whether the value is present.

---

### random

```cpp
template <typename UniformRandomBitGenerator>
const T& random(UniformRandomBitGenerator& generator) const;
```

Returns a reference to a randomly selected value, weighted by each value's weight.

The random engine is supplied by the caller rather than owned by the container. This keeps the container deterministic to test and lets callers choose their own random policy.

Throws `std::runtime_error` if the set is empty.

---

### clear

```cpp
void clear();
```

Removes all values and resets `total_weight()` to zero.

---

### State Inspection

```cpp
bool empty() const;
std::size_t size() const;
weight_type total_weight() const;
```

---

### Value Semantics

```cpp
weighted_set(const weighted_set&) = default;
weighted_set& operator=(const weighted_set&) = default;

weighted_set(weighted_set&&) = default;
weighted_set& operator=(weighted_set&&) = default;
```

`weighted_set` has ordinary value semantics. Copying creates another set with the same values and weights. Moving transfers the stored values, index, and total weight.

---

### Iteration

```cpp
auto begin() const;
auto end() const;
```

Iterates over entries containing:

```cpp
struct entry {
    T value;
    weight_type weight;
};
```

Iteration order is not part of the API contract.

---

## Behaviour Summary

| Operation    | Missing Value | Duplicate Value | Zero Weight | Empty Set |
|-------------|---------------|-----------------|-------------|-----------|
| insert      | inserts       | returns false   | throws      | inserts   |
| erase       | returns false | removes         | n/a         | returns false |
| set_weight  | returns false | updates         | throws      | returns false |
| weight_of   | throws        | returns weight  | n/a         | throws    |
| random      | n/a           | may return it   | n/a         | throws    |

---

## Design Notes

`weighted_set` intentionally uses one concrete weight type:

```cpp
using weight_type = std::uint64_t;
```

This keeps the rules simple:

- weights are positive integers
- zero is invalid
- negative weights are impossible
- floating-point precision, `NaN`, and infinity are avoided
- random selection can use an integer distribution over `[1, total_weight()]`

The caller supplies the random engine:

```cpp
std::mt19937 generator(123);
set.random(generator);
```

This avoids hidden random state inside the container and makes tests repeatable.

The mutation API is intentionally explicit:

```cpp
insert(value, weight);
set_weight(value, weight);
erase(value);
```

This is a little larger than a single "set weight" operation that inserts, updates, and removes when given zero. The tradeoff is clarity: insertion, weight updates, and removal are different operations with different failure modes. Keeping them separate also preserves the simple invariant that every stored weight is positive; `0` is always invalid, never a hidden removal command.

---

## Misuse Resistance

Common invalid uses are rejected directly:

- inserting or assigning a zero weight throws `std::invalid_argument`
- overflowing the total weight throws `std::overflow_error`
- reading a missing weight throws `std::out_of_range`
- choosing from an empty set throws `std::runtime_error`

Duplicate insertion is treated as an expected condition and returns `false`.

---

## Implementation Notes

The implementation stores entries in a `std::vector` and keeps an `std::unordered_map` from value to vector index.

This gives:

- O(1) average `contains`
- O(1) average `weight_of`
- O(1) average `set_weight`
- O(1) average `erase` by swapping with the last entry
- O(n) `random`, using a cumulative weight scan

The O(n) random scan is deliberate for the first version. It is simple, easy to audit, and suitable for moderate set sizes. If repeated sampling from large mostly-static sets becomes important, an alias-table or cached-prefix variant could be added later as a separate optimization.

---

## Requirements

`T` must be:

- hashable by `Hash`
- equality-comparable by `KeyEqual`
- nothrow move assignable

Custom hashing and equality can be supplied through the template parameters:

```cpp
obz::weighted_set<my_type, my_hash, my_equal> set;
```

---

## When to Use

Use `weighted_set` when:

- each item should appear at most once
- items have integer relative probabilities
- deterministic testing of random selection matters
- simple O(n) sampling is acceptable

Typical use cases:

- loot tables
- weighted routing
- randomized choices in simulations
- priority-biased selection

---

## When Not to Use

Do not use when:

- duplicate values with separate weights are required
- weights must be fractional
- cryptographic randomness is required
- extremely large sets need very fast repeated sampling
- stable iteration order is required

---

## Future Improvements

Potential extensions:

- `reserve`
- alias-table optimized sampler
- cached prefix-weight sampler
- `insert_or_assign`
- transparent lookup for compatible hash/equality types

---

## License

Part of the `obz` project.
