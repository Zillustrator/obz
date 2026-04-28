# obz::blocking_queue

A thread-safe blocking queue for producer-consumer scenarios.

This container provides safe concurrent access for multiple producers and consumers, with support for blocking waits and graceful shutdown.

---

## Features

- Thread-safe push and pop operations  
- Blocking `wait_and_pop` for efficient waiting  
- Non-blocking `try_pop`  
- Graceful shutdown via `close()`  
- Exception on push after close  
- FIFO ordering  
- Header-only implementation  

---

## Usage

```cpp
#include <obz/blocking_queue.hpp>

obz::blocking_queue<int> queue;

queue.push(42);

int value = 0;
if (queue.wait_and_pop(value)) {
    // value == 42
}
```

---

## Producer / Consumer Example

```cpp
#include <obz/blocking_queue.hpp>
#include <thread>
#include <iostream>

obz::blocking_queue<int> queue;

void producer() {
    for (int i = 0; i < 5; ++i) {
        queue.push(i);
    }
    queue.close();
}

void consumer() {
    int value;
    while (queue.wait_and_pop(value)) {
        std::cout << value << std::endl;
    }
}

int main() {
    std::thread p(producer);
    std::thread c(consumer);

    p.join();
    c.join();
}
```

---

## API

### push

```cpp
void push(const T& value);
void push(T&& value);
```

Adds an element to the queue.

Throws `std::runtime_error` if the queue has been closed.

---

### emplace

```cpp
template <typename... Args>
void emplace(Args&&... args);
```

Constructs an element in-place at the back of the queue.

---

### wait_and_pop

```cpp
bool wait_and_pop(T& value);
```

Blocks until:
- an element is available, or  
- the queue is closed  

Returns:
- `true` if a value was retrieved  
- `false` if the queue is closed and empty  

---

### try_pop

```cpp
bool try_pop(T& value);
```

Attempts to retrieve a value without blocking.

Returns `true` if successful, `false` otherwise.

---

### close

```cpp
void close();
```

Marks the queue as closed:

- Wakes all waiting threads  
- Prevents further pushes  
- Allows consumers to exit cleanly  

---

### state inspection

```cpp
bool empty() const;
std::size_t size() const;
bool closed() const;
```

---

## Threading Model

- Multiple producers and consumers are supported  
- Internally synchronized using:
  - `std::mutex`
  - `std::condition_variable`  
- Blocking operations use `std::unique_lock`  
- Non-blocking operations use `std::lock_guard`  

---

## Design Notes

- FIFO ordering is preserved  
- `wait_and_pop` uses a predicate to handle spurious wakeups  
- `close()` enables clean shutdown of worker threads  
- Destructor calls `close()` to avoid deadlocks during teardown  

---

## When to Use

Use `blocking_queue` when:

- implementing producer-consumer pipelines  
- coordinating worker threads  
- building message passing systems  
- prototyping concurrent systems  

---

## When Not to Use

Consider alternatives when:

- ultra-low latency is required (lock-free structures may be preferred)  
- contention is extremely high  
- bounded queues or backpressure are required  

---

## Future Improvements

Potential extensions:

- bounded capacity  
- timed wait operations  
- lock-free variant  
- custom allocator support  

---

## License

Part of the `obz` project.