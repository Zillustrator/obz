# obz::bounded_blocking_queue

A thread-safe bounded blocking queue for producer-consumer scenarios.

Unlike `obz::blocking_queue`, this container enforces a fixed capacity and provides backpressure by blocking producers when the queue is full.

---

## Features

- Thread-safe push and pop operations  
- Fixed capacity (bounded size)  
- Blocking `push` when full  
- Blocking `wait_and_pop` when empty  
- Non-blocking `try_push` and `try_pop`  
- Graceful shutdown via `close()`  
- FIFO ordering  
- Header-only implementation  

---

## Usage

```cpp
#include <obz/bounded_blocking_queue.hpp>

obz::bounded_blocking_queue<int> queue(2);

queue.push(1);
queue.push(2);

// queue is now full, next push will block
```

---

## Producer / Consumer Example

```cpp
#include <obz/bounded_blocking_queue.hpp>
#include <thread>
#include <iostream>

obz::bounded_blocking_queue<int> queue(2);

void producer() {
    for (int i = 0; i < 5; ++i) {
        queue.push(i);  // blocks if queue is full
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

- Blocks if the queue is full  
- Throws `std::runtime_error` if the queue is closed  

---

### emplace

```cpp
template <typename... Args>
void emplace(Args&&... args);
```

Constructs an element in-place at the back of the queue.

- Blocks if the queue is full  

---

### try_push

```cpp
bool try_push(const T& value);
bool try_push(T&& value);
```

Attempts to add an element without blocking.

Returns:
- `true` if successful  
- `false` if the queue is full or closed  

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

Returns:
- `true` if successful  
- `false` if the queue is empty  

---

### close

```cpp
void close();
```

Marks the queue as closed:

- Wakes all waiting threads  
- Prevents further pushes  
- Allows producers and consumers to exit cleanly  

---

### state inspection

```cpp
bool empty() const;
bool full() const;
std::size_t size() const;
std::size_t capacity() const;
bool closed() const;
```

---

## Threading Model

- Multiple producers and consumers are supported  
- Internally synchronized using:
  - `std::mutex`
  - `std::condition_variable`  
- Producers wait on `not_full` condition  
- Consumers wait on `not_empty` condition  

---

## Behaviour Summary

| Operation        | When Queue is Empty | When Queue is Full | When Closed |
|-----------------|--------------------|--------------------|-------------|
| `push`          | succeeds           | blocks             | throws      |
| `try_push`      | succeeds           | returns false      | returns false |
| `wait_and_pop`  | blocks             | succeeds           | returns false if empty |
| `try_pop`       | returns false      | succeeds           | succeeds if not empty |

---

## Design Notes

- Capacity is fixed at construction  
- FIFO ordering is preserved  
- Separate condition variables are used for:
  - `not_empty`
  - `not_full`  
- `close()` ensures clean shutdown of blocked threads  
- Destructor calls `close()` to avoid deadlocks  

---

## When to Use

Use `bounded_blocking_queue` when:

- you need backpressure in a pipeline  
- producers may outpace consumers  
- you want to limit memory usage  
- building thread pools or message queues  

---

## When Not to Use

Consider alternatives when:

- unbounded growth is acceptable → use `blocking_queue`  
- ultra-low latency is required → consider lock-free structures  
- single-threaded usage → simpler containers are sufficient  

---

## Future Improvements

Potential extensions:

- timed wait operations (`wait_for`, `wait_until`)  
- configurable blocking policies  
- lock-free bounded queue  
- custom allocator support  

---

## License

Part of the `obz` project.