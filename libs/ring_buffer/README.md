# obz::ring_buffer

A fixed-capacity circular buffer providing fast FIFO storage with no dynamic allocation after construction.

The `ring_buffer` is a lightweight container designed for predictable performance and constant memory usage.

---

## Features

- Fixed capacity (no resizing)  
- Constant-time push and pop operations  
- FIFO ordering  
- Wraparound (circular) storage  
- No dynamic allocation after construction  
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

- overwrite-on-full mode  
- iterator support  
- support for non-default-constructible types  
- lock-free SPSC ring queue built on top of this  
- custom allocator support  

---

## License

Part of the `obz` project.
