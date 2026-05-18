# obz::thread_affinity

Small cross-platform helpers for pinning the current thread to a logical CPU when the platform supports it.

`thread_affinity` solves one narrow problem: callers sometimes need to ask the operating system to keep a latency-sensitive thread on one logical CPU. The library keeps that platform-specific operation behind a small public API and reports unsupported platforms explicitly.

---

## Features

- Current-thread CPU affinity only
- Simple `cpu_index` type
- Throwing and non-throwing pin APIs
- `std::system_error` for operating-system failures
- Linux backend using `pthread_setaffinity_np`
- Windows backend using `SetThreadAffinityMask`
- Unsupported platforms compile and report unsupported

---

## Usage

```cpp
#include <obz/thread_affinity.hpp>

obz::pin_current_thread_to_cpu(2);
```

---

## Basic Example

```cpp
#include <obz/thread_affinity.hpp>

#include <thread>

int main() {
    std::thread worker([] {
        if (obz::thread_affinity_supported()) {
            obz::pin_current_thread_to_cpu(2);
        }

        // latency-sensitive work
    });

    worker.join();
}
```

---

## API

### cpu_index

```cpp
using cpu_index = std::uint32_t;
```

Represents a logical CPU index, not necessarily a physical core index.

---

### thread_affinity_supported

```cpp
bool thread_affinity_supported() noexcept;
```

Returns whether this build has a platform backend for thread affinity.

This reports compile-time platform support. A supported platform can still reject a specific CPU index or affinity request at runtime.

---

### try_pin_current_thread_to_cpu

```cpp
bool try_pin_current_thread_to_cpu(cpu_index cpu) noexcept;
```

Attempts to pin the current thread to `cpu`.

Returns:
- `true` if the affinity request succeeded
- `false` if the platform is unsupported, the CPU index is invalid, or the operating system rejects the request

Use this when affinity is an optional optimization.

---

### pin_current_thread_to_cpu

```cpp
void pin_current_thread_to_cpu(cpu_index cpu);
```

Pins the current thread to `cpu`, or throws if the request cannot be completed.

Throws:
- `std::invalid_argument` when the CPU index cannot be represented by the platform backend
- `std::system_error` when the platform is unsupported or the operating system rejects the request

Use this when affinity is required for the caller's execution model.

---

## Behaviour Summary

| Situation | `try_pin_current_thread_to_cpu` | `pin_current_thread_to_cpu` |
|-----------|----------------------------------|------------------------------|
| supported platform and accepted CPU | returns `true` | returns normally |
| unsupported platform | returns `false` | throws `std::system_error` |
| invalid CPU index | returns `false` | throws |
| operating-system failure | returns `false` | throws `std::system_error` |

---

## Design Notes

The library pins only the current thread. It intentionally does not expose process-level affinity because process affinity changes the execution limits for all threads in the process and is easier to misuse from a small utility library.

The public API talks about logical CPUs rather than physical cores. Operating systems schedule threads on logical CPUs, and logical CPU numbering may include hyper-threaded siblings or other topology details.

The non-throwing API exists because affinity is often an optimization. Many programs should continue running if the operating system rejects the request.

---

## Misuse Resistance

Common mistakes are handled deliberately:

- unsupported platforms compile cleanly and report unsupported
- invalid CPU indexes fail instead of shifting past the platform affinity mask
- operating-system failures are translated to `std::system_error`
- process affinity is left out of the API

---

## Implementation Notes

Platform-specific code lives in separate implementation files selected by CMake:

- Linux builds compile `src/platform/linux_thread_affinity.cpp`
- Windows builds compile `src/platform/win32_thread_affinity.cpp`
- other platforms compile `src/platform/unsupported_thread_affinity.cpp`

The public header does not include `pthread.h`, `sched.h`, or `windows.h`.

Linux support uses `pthread_setaffinity_np`, which is a Linux extension rather than portable POSIX. macOS currently uses the unsupported backend.

Windows support uses `SetThreadAffinityMask`, which is limited to the current processor group on systems with more than one processor group.

---

## When to Use

Use `thread_affinity` when:

- a hot worker thread should avoid moving between logical CPUs
- benchmark code needs less scheduler movement
- a polling loop or low-latency thread benefits from explicit placement
- failing to pin can be treated as either an error or an optional optimization

---

## When Not to Use

Avoid `thread_affinity` when:

- the workload has not been measured
- normal operating-system scheduling is good enough
- the program needs portable behavior on every platform
- CPU topology, NUMA placement, processor groups, or real-time scheduling must be managed

---

## Future Improvements

Potential extensions:

- query the number of logical CPUs
- pin to a set of CPUs
- expose the current affinity mask
- add explicit Windows processor-group support
- add Linux tests in CI that validate a real affinity change

---

## License

Part of the `obz` project.
