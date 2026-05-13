# obz::mpsc_ring_queue

A fixed-capacity, lock-free multi-producer single-consumer (MPSC) ring queue.

This container is designed for high-performance scenarios where many producer threads send data to exactly one consumer thread without locks, condition variables, or dynamic allocation.

---

## Features

- Lock-free multi-producer single-consumer design
- Fixed capacity (compile-time)
- No dynamic allocation
- FIFO ordering by successful producer reservation
- Supports non-default-constructible types
- Cache-line aware producer and consumer indices
- Header-only implementation

---

## Usage

```cpp
#include <obz/mpsc_ring_queue.hpp>

obz::mpsc_ring_queue<int, 1024> queue;

queue.try_push(42);

int value = 0;

if (queue.try_pop(value)) {
    // value == 42
}
```

---

## Producer / Consumer Example

```cpp
#include <obz/mpsc_ring_queue.hpp>

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    obz::mpsc_ring_queue<int, 1024> queue;

    constexpr int producer_count = 4;
    constexpr int values_per_producer = 1000;

    std::atomic<int> producers_done{0};
    std::vector<std::thread> producers;

    for (int producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([&, producer] {
            for (int value = 0; value < values_per_producer; ++value) {
                const int item = producer * values_per_producer + value;

                while (!queue.try_push(item)) {
                    std::this_thread::yield();
                }
            }

            producers_done.fetch_add(1, std::memory_order_release);
        });
    }

    std::thread consumer([&] {
        int value = 0;

        while (producers_done.load(std::memory_order_acquire) != producer_count ||
               !queue.empty()) {
            if (queue.try_pop(value)) {
                std::cout << value << '\n';
            } else {
                std::this_thread::yield();
            }
        }
    });

    for (auto& producer : producers) {
        producer.join();
    }

    consumer.join();
}
```

---

## API

### try_push

```cpp
bool try_push(const T& value);
bool try_push(T&& value);
```

Attempts to add an element to the queue.

Returns:
- `true` if successful
- `false` if the queue is full

`T` must be nothrow constructible from the provided value.

---

### try_emplace

```cpp
template <typename... Args>
bool try_emplace(Args&&... args);
```

Constructs an element in-place at the back of the queue.

Returns `false` if the queue is full.

`T` must be nothrow constructible from `Args...`. This matters because a producer claims a FIFO position before constructing the object. If construction could throw after the claim, the queue could be left with an unpublished hole that blocks the single consumer.

---

### try_pop

```cpp
bool try_pop(T& value);
```

Attempts to remove the oldest currently available element.

Returns:
- `true` if successful
- `false` if no element is currently available to pop

`T` must be nothrow move assignable. After the consumer observes a published slot, it needs to move the value out, destroy the stored object, advance the read index, and release the slot back to producers. If move assignment could throw, the queue could be left in a partially completed pop operation.

Only one consumer thread may call `try_pop`.

---

### State Inspection

```cpp
bool empty() const;
bool full() const;
std::size_t size() const;
constexpr std::size_t capacity() const;
```

`empty()` means no element is currently available to pop. It does not mean no producer has reserved a slot. A producer may have claimed a position but not yet published the constructed object.

`size()` is an approximate snapshot. It is computed from the producer and consumer indices, so it includes claimed but not yet published slots. An exact available count is not trivial in MPSC because producers mutate per-cell sequence state concurrently.

---

### clear

```cpp
void clear();
```

Destroys currently published elements.

`clear()` requires external synchronization. It is not safe to call while producers are active, because a producer may have claimed a cell that has not been published yet.

The destructor calls `clear()` and assumes all producer and consumer threads have stopped using the queue.

---

## Behaviour Summary

| Operation   | When Empty     | When Full     |
|------------|----------------|---------------|
| try_push   | succeeds       | returns false |
| try_pop    | returns false  | succeeds      |

---

## Threading Model

- Multiple producer threads may call `try_push` and `try_emplace`
- Exactly one consumer thread may call `try_pop`
- `empty`, `full`, and `size` are snapshots
- `clear` and destruction require external synchronization
- Multiple consumers result in undefined behaviour

The queue is implemented using:

- `std::atomic<std::size_t>` indices
- Per-cell `std::atomic<std::size_t>` sequence numbers
- Acquire/release memory ordering on cell publication and reuse
- Raw storage with manual object lifetime management

---

## Synchronization Model

The queue uses two related synchronization layers:

1. `write_index_` assigns unique logical positions to producers.
2. Each cell's `sequence` value describes whether that physical slot is free, published, or waiting for reuse.

`write_index_` is the contention point between producers. Producers use `compare_exchange_weak` to claim exactly one logical write position. Winning that CAS gives a producer ownership of one slot, but it does not publish the object to the consumer.

`read_index_` is not a contention point because there is exactly one consumer. It is the consumer's FIFO cursor: the consumer only attempts to pop the cell at `read_index_ % Capacity`.

The per-cell `sequence` value is the state gate between producers and the consumer:

- Producers use it to decide whether a physical slot is free for a specific write position.
- The consumer uses it to decide whether the next FIFO slot has actually been published.
- Producers use it again after wrap-around to avoid reusing storage before the consumer has destroyed the previous object.

This separation is the key difference from `spsc_ring_queue`. In SPSC, producer and consumer cursors are enough. In MPSC, producers also need a per-slot publication state because a producer can claim a write position and then be delayed before constructing the object.

---

## Design Notes

`mpsc_ring_queue<T, Capacity>` is a compile-time-capacity queue:

```cpp
obz::mpsc_ring_queue<int, 1024> queue;
```

The capacity is part of the type so storage can be embedded directly in the queue object with no allocation.

Internally, the queue is a circular buffer of `Capacity` cells. Each cell contains raw storage for `T` and a sequence number.

At initialization:

```cpp
cell[0].sequence == 0
cell[1].sequence == 1
cell[2].sequence == 2
```

For a producer at write position `N`:

```cpp
cell.sequence == N
```

means the cell is free and may be claimed.

After construction, the producer publishes the object with:

```cpp
cell.sequence = N + 1
```

For the consumer at read position `N`:

```cpp
cell.sequence == N + 1
```

means the object is ready to pop.

After destruction, the consumer releases the cell for the next wrap-around:

```cpp
cell.sequence = N + Capacity
```

This per-cell sequence number is what prevents producers from overwriting unread data and prevents the consumer from reading unconstructed storage.

The sequence value has a simple lifecycle for logical position `N`:

| Sequence value | Meaning |
|----------------|---------|
| `N` | The cell is free for the producer that wants to write position `N`. |
| `N + 1` | The producer has constructed the object and published it to the consumer. |
| `N + Capacity` | The consumer has destroyed the object and released the cell for the next wrap-around. |

The queue uses monotonically increasing `std::size_t` logical positions and sequence values. Physical storage is selected with modulo indexing, but correctness depends on the logical counters not wrapping so far that old and new sequence states become indistinguishable. In normal long-running applications this would require an extremely large number of operations, but it is still part of the queue's correctness contract.

`empty()`, `full()`, and `size()` are concurrent snapshots. In particular, `size()` includes producer-reserved slots that may not have been published yet. Treat these functions as diagnostics or polling hints; `try_push` and `try_pop` are the operations that establish whether a push or pop actually happened.

---

## Producer Coordination

Producers coordinate by reserving positions from `write_index_` using `compare_exchange_weak`.

The `diff` calculation compares the cell sequence number against the producer's desired write position:

```cpp
diff = cell.sequence - write
```

Meaning:

- `diff == 0`: the cell is free for this write position
- `diff < 0`: the queue is full at this position
- `diff > 0`: another producer moved the write position, so reload and retry

The successful CAS only assigns ownership of the cell. The constructed object is not visible to the consumer until the producer stores the new sequence value.

---

## Ordering

FIFO ordering is based on successful producer reservations.

If producer A reserves position `10` and producer B reserves position `11`, the consumer must pop position `10` first. If producer A is delayed before publishing, producer B's later value may be ready internally, but the consumer cannot skip over position `10`.

This preserves queue order, but it means one delayed producer can temporarily block later published values.

---

## Memory Ordering

The per-cell sequence number carries the object visibility guarantees.

Producer side:

```cpp
std::construct_at(cell.slot(), ...);
cell.sequence.store(write + 1, std::memory_order_release);
```

Consumer side:

```cpp
auto sequence = cell.sequence.load(std::memory_order_acquire);
```

The release store publishes the constructed object. The consumer's acquire load makes that construction visible before it reads the object.

After popping, the consumer destroys the object and releases the cell:

```cpp
std::destroy_at(cell.slot());
cell.sequence.store(read + Capacity, std::memory_order_release);
```

Producers load the sequence with acquire before reusing a cell. This ensures a producer does not construct a new object in the same storage before the previous destruction is visible.

The `write_index_` CAS uses relaxed ordering because it only assigns a unique position to one producer. It does not publish object contents. Object publication is handled by the cell sequence release store.

`read_index_` also uses relaxed stores in `try_pop` because it is written only by the single consumer. Slot reuse visibility is not carried by `read_index_`; it is carried by the release store to the cell's `sequence` value after destruction.

---

## Cache-Line Considerations

The read and write indices are placed on separate cache lines using:

```cpp
alignas(std::hardware_destructive_interference_size)
```

This helps reduce false sharing between producers updating `write_index_` and the consumer updating `read_index_`.

---

## Performance Characteristics

- No dynamic allocation after construction
- Constant-time successful push and pop
- Lock-free producer coordination
- Suitable for low-latency and high-throughput message passing

Under high producer contention, `try_push` may retry its CAS several times before succeeding or discovering that the queue is full.

---

## When to Use

Use `mpsc_ring_queue` when:

- multiple producers send work to one consumer
- fixed capacity is acceptable
- non-blocking `try_push` / `try_pop` semantics are desired
- predictable allocation-free behaviour matters

Typical use cases:

- logging pipelines
- telemetry ingestion
- fan-in message passing
- event queues with one processing thread

---

## When Not to Use

Do not use when:

- multiple consumers are required
- blocking behaviour is needed (use `blocking_queue`)
- producer backpressure with waiting is required (use `bounded_blocking_queue`)
- element construction or pop move assignment may throw

---

## Future Improvements

Potential extensions:

- batch push/pop operations
- wait/notify integration
- power-of-two capacity optimization
- alternative pop API that constructs into optional-like storage

---

## License

Part of the `obz` project.
