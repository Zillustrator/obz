# obz

`obz` is a collection of reusable modern C++ libraries developed under the `obz` namespace.

The goal of this project is to provide small, focused, and independent C++20 libraries that can be reused across multiple systems and applications.

---

## Goals

- Modern C++20  
- Cross-platform design  
- Small, independent libraries  
- Clean and minimal APIs  
- Reusable across multiple projects  
- Suitable for future packaging (CMake packages, NuGet, GitHub releases)

---

## Repository Structure

```
obz/
├── CMakeLists.txt
├── libs/
│   └── <library_name>/
│       ├── CMakeLists.txt
│       ├── README.md
│       ├── include/
│       │   └── obz/
│       │       └── <library_name>.hpp
│       └── tests/
│           └── <library_name>_tests.cpp
├── tests/
│   └── CMakeLists.txt
└── README.md
```

---

## Libraries

Each library lives under `libs/<library_name>` and is documented in its own `README.md`.

The root README intentionally avoids duplicating per-library APIs and examples. This keeps the repository overview stable as new libraries are added.

Capacity is expressed according to the library's purpose:

- General-purpose containers such as `ring_buffer<T>` use runtime capacity selected at construction.
- Lock-free ring queues such as `spsc_ring_queue<T, Capacity>` and `mpsc_ring_queue<T, Capacity>` use compile-time capacity so storage is embedded directly in the queue object.

Typical library layout:

```text
libs/<library_name>/
├── CMakeLists.txt
├── README.md
├── include/obz/<library_name>.hpp
└── tests/<library_name>_tests.cpp
```

---

## Requirements

- C++20 compatible compiler  
- CMake 3.21 or later  

Test dependencies are fetched automatically when tests are enabled.

---

## Build

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

---

## Build Options

### Build With Tests (Default)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

---

### Build Without Tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DOBZ_BUILD_TESTS=OFF
cmake --build build
```

---

### Build Without Examples

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DOBZ_BUILD_EXAMPLES=OFF
cmake --build build
```

---

### Build Without Tests Or Examples

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DOBZ_BUILD_TESTS=OFF \
  -DOBZ_BUILD_EXAMPLES=OFF
cmake --build build
```

---

## Running Tests

```bash
ctest --test-dir build --output-on-failure
```

---

## Namespace

All public APIs are exposed under the `obz` namespace:

```cpp
obz::<library_name> object;
```

---

## CMake Integration

Libraries are exposed as namespaced CMake targets:

```cmake
target_link_libraries(my_app
    PRIVATE
        obz::<library_name>
)
```

### Fetch From Git

Use `FetchContent` when a project should build a pinned ObzLib revision directly
from source:

```cmake
include(FetchContent)

set(OBZ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OBZ_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    obz
    GIT_REPOSITORY https://github.com/Zillustrator/obz.git
    GIT_TAG <release-tag-or-commit>
)

FetchContent_MakeAvailable(obz)

target_link_libraries(my_app
    PRIVATE
        obz::blocking_queue
        obz::transport
)
```

Pin a release tag or commit rather than tracking `main`.

### Install And Find The Package

Install ObzLib when consumers should build independently from its source tree:

```bash
cmake -S . -B build -DOBZ_BUILD_TESTS=OFF
cmake --build build
cmake --install build --prefix /path/to/obz-install
```

The consumer can then use the installed CMake package:

```cmake
find_package(obz 0.1 CONFIG REQUIRED)

target_link_libraries(my_app
    PRIVATE
        obz::blocking_queue
        obz::transport
)
```

Configure the consumer with the install prefix:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/obz-install
```

The independent project under `tests/consumer` verifies both source dependency
and installed-package consumption. It checks that headers, compiled libraries,
package metadata, and public target names work without relying on ObzLib's
internal build structure.

---

## Status

Early development.

The structure and patterns are being established before expanding the library set.
