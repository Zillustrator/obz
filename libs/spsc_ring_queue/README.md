# obz::spsc_ring_queue

A fixed-capacity, lock-free single-producer single-consumer (SPSC) ring queue.

This container is designed for high-performance scenarios where exactly one thread produces data and one thread consumes it, without the overhead of locks or condition variables.

---

## Features

- Lock-free single-producer single-consumer design  
- Fixed capacity (compile-time)  
- No dynamic allocation  
- Constant-time push and pop  
- FIFO ordering  
- Supports non-default-constructible types  
- Cache-line aware design to minimise false sharing  
- Header-only implementation  

---

## Usage

```cpp
#include <obz/spsc_ring_queue.hpp>

obz::spsc_ring_queue<int, 1024> queue;

queue.try_push(42);

int value = 0;

if (queue.try_pop(value)) {
    // value == 42
}
```

---

## Producer / Consumer Example

```cpp
#include <obz/spsc_ring_queue.hpp>
#include <thread>
#include <atomic>
#include <iostream>

int main() {
    obz::spsc_ring_queue<int, 1024> queue;

    std::atomic<bool> done = false;

    std::thread producer([&] {
        for (int i = 0; i < 10; ++i) {
            while (!queue.try_push(i)) {
                std::this_thread::yield();
            }
        }
        done = true;
    });

    std::thread consumer([&] {
        int value;
        while (!done || !queue.empty()) {
            if (queue.try_pop(value)) {
                std::cout << value << std::endl;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
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

---

### try_emplace

```cpp
template <typename... Args>
bool try_emplace(Args&&... args);
```

Constructs an element in-place at the back of the queue.

Returns `false` if the queue is full.

---

### try_pop

```cpp
bool try_pop(T& value);
```

Attempts to remove the oldest element.

Returns:
- `true` if successful  
- `false` if the queue is empty  

---

### State Inspection

```cpp
bool empty() const;
bool full() const;
std::size_t size() const;
constexpr std::size_t capacity() const;
```

---

## Behaviour Summary

| Operation   | When Empty     | When Full     |
|------------|----------------|---------------|
| try_push   | succeeds       | returns false |
| try_pop    | returns false  | succeeds      |

---

## Threading Model

- Exactly **one producer thread** may call `try_push`  
- Exactly **one consumer thread** may call `try_pop`  
- Multiple producers or consumers result in undefined behaviour  

The queue is implemented using:

- `std::atomic<std::size_t>` indices (cache-line separated)  
- Acquire/release memory ordering  
- Raw storage with manual object lifetime management  

---

## Cache-Line Considerations

The read and write indices are placed on separate cache lines using:

```cpp
alignas(std::hardware_destructive_interference_size)
```

This prevents **false sharing** between the producer and consumer threads.

Without this separation, updates to `write_index` by the producer and `read_index` by the consumer could invalidate each other's cache lines, significantly degrading performance under contention.

This design ensures that:

- the producer primarily writes to one cache line  
- the consumer primarily writes to another  
- cache coherency traffic is minimised  

This is particularly important in low-latency and high-throughput systems.

---

## Design Notes

- `spsc_ring_queue<T, Capacity>` is a compile-time-capacity queue:

```cpp
obz::spsc_ring_queue<int, 1024> queue;
```

- The capacity is part of the type so storage can be embedded directly in the queue object with no allocation.

- Internally uses a circular buffer of size `Capacity`  
- Read and write indices grow monotonically and wrap via modulo indexing  
- Full condition is determined by:

```
write_index - read_index == Capacity
```

- Empty condition is:

```
write_index == read_index
```

- No locks or condition variables are used  

---

## Performance Characteristics

- No dynamic allocation after construction  
- Minimal synchronization overhead (atomics only, cache-line optimised)  
- Suitable for low-latency and high-throughput systems  

---

## When to Use

Use `spsc_ring_queue` when:

- exactly one producer and one consumer exist  
- low latency is required  
- high throughput is required  
- predictable performance is important  
- avoiding locks is desirable  

Typical use cases:

- market data pipelines  
- logging systems  
- telemetry pipelines  
- message passing between threads  

---

## When Not to Use

Do not use when:

- multiple producers or consumers are required  
- blocking behaviour is needed (use `blocking_queue`)  
- backpressure is required (use `bounded_blocking_queue`)  

---

## Future Improvements

Potential extensions:

- multi-producer or multi-consumer variants  
- wait/notify integration  
- batch push/pop operations  

---

## License

Part of the `obz` project.
