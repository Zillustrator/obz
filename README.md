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

### Build with tests (default)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

---

### Build without tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DOBZ_BUILD_TESTS=OFF
cmake --build build
```

---

### Build without examples

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DOBZ_BUILD_EXAMPLES=OFF
cmake --build build
```

---

### Build without tests or examples

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

---

## Status

Early development.

The structure and patterns are being established before expanding the library set.
