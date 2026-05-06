# obz::ring_buffer

A runtime-capacity circular buffer providing fast FIFO storage with no dynamic allocation after construction.

The `ring_buffer` is a lightweight single-threaded container designed for predictable performance and constant memory usage after its initial allocation.

---

## Features

- Runtime capacity selected at construction  
- Fixed capacity (no resizing)  
- Constant-time push and pop operations  
- FIFO ordering  
- Wraparound (circular) storage  
- One allocation at construction and no dynamic allocation after construction  
- Supports non-default-constructible types  
- Header-only implementation  

---

## Usage

```cpp
#include <obz/ring_buffer.hpp>

obz::ring_buffer<int> buffer(3);

buffer.push_back(1);
buffer.push_back(2);
buffer.push_back(3);

int value = 0;

buffer.pop_front(value); // value = 1
```

---

## Basic Example

```cpp
#include <obz/ring_buffer.hpp>
#include <iostream>

int main() {
    obz::ring_buffer<int> buffer(2);

    buffer.push_back(10);
    buffer.push_back(20);

    if (!buffer.push_back(30)) {
        std::cout << "buffer is full\n";
    }

    int value;

    while (buffer.pop_front(value)) {
        std::cout << value << std::endl;
    }
}
```

---

## API

### push_back

```cpp
bool push_back(const T& value);
bool push_back(T&& value);
```

Adds an element to the buffer.

Returns:
- `true` if successful  
- `false` if the buffer is full  

---

### emplace_back

```cpp
template <typename... Args>
bool emplace_back(Args&&... args);
```

Constructs an element in-place at the back of the buffer.

Returns `false` if the buffer is full.

---

### pop_front

```cpp
bool pop_front(T& value);
```

Removes the oldest element and stores it in `value`.

Returns:
- `true` if successful  
- `false` if the buffer is empty  

---

### front

```cpp
T& front();
const T& front() const;
```

Returns a reference to the oldest element.

Throws `std::runtime_error` if the buffer is empty.

---

### clear

```cpp
void clear();
```

Removes all elements from the buffer.

After `clear()`, the buffer can be reused with the same capacity.

---

### State Inspection

```cpp
bool empty() const;
bool full() const;
std::size_t size() const;
std::size_t capacity() const;
```

---

## Behaviour Summary

| Operation      | When Empty      | When Full        |
|----------------|----------------|------------------|
| `push_back`    | succeeds       | returns false    |
| `pop_front`    | returns false  | succeeds         |
| `front`        | throws         | succeeds         |

---

## Threading Model

`ring_buffer` is not internally synchronized.

Use external synchronization if multiple threads need to access the same buffer, or use a queue type such as `blocking_queue` or `bounded_blocking_queue` when thread-safe producer-consumer behaviour is required.

---

## Design Notes

- `ring_buffer<T>` is a runtime-capacity container. The capacity is selected when the buffer is constructed:

```cpp
obz::ring_buffer<int> buffer(1024);
```

- This makes it suitable as a general-purpose circular buffer when the size may come from configuration or runtime workload decisions.

- Internally uses a circular buffer with:
  - `head` index (read position)  
  - `tail` index (write position)  
  - `size` counter  

- Wraparound is handled via modulo indexing  

- No resizing — capacity is fixed at construction  

- This implementation uses raw allocated storage, so elements are constructed only when inserted and destroyed when removed.

---

## Misuse Resistance

- Construction with zero capacity throws `std::invalid_argument`.
- Pushing to a full buffer returns `false` instead of overwriting existing values.
- Popping from an empty buffer returns `false`.
- Accessing `front()` on an empty buffer throws `std::runtime_error`.
- Copy and move operations are disabled so ownership of the raw storage remains simple and explicit.

---

## Implementation Notes

The buffer allocates raw storage for `capacity` elements during construction.

Elements are not default-constructed. `emplace_back` constructs an element into the current tail slot with `std::construct_at`, and `pop_front` destroys the current head slot with `std::destroy_at`.

The `head`, `tail`, and `size` members define the invariant:

- `head` points at the next element to read
- `tail` points at the next slot to write
- `size` tracks how many constructed elements are currently stored

Modulo arithmetic wraps `head` and `tail` back to the start of the allocation.

---

## When to Use

Use `ring_buffer` when:

- you need fixed-size buffering  
- predictable memory usage is required  
- implementing sliding windows or recent-history buffers  
- buffering data between stages of a system  
- building higher-level queue structures  

---

## When Not to Use

Consider alternatives when:

- dynamic resizing is required  
- thread safety is needed (use `blocking_queue` or `bounded_blocking_queue`)  
- overwrite-on-full semantics are desired (not implemented in this version)  

---

## Future Improvements

Potential extensions:

- iterator support
- allocator-aware storage

---

## License

Part of the `obz` project.
