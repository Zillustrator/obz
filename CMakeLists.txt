cmake_minimum_required(VERSION 3.21)

# ------------------------------------------------------------------------------
# Project
# ------------------------------------------------------------------------------

project(obz VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(OBZ_BUILD_TESTS "Build obz tests" ON)
option(OBZ_BUILD_EXAMPLES "Build obz examples" ON)

# ------------------------------------------------------------------------------
# Libraries
# ------------------------------------------------------------------------------

add_subdirectory(libs/blocking_queue)
add_subdirectory(libs/bounded_blocking_queue)
add_subdirectory(libs/ring_buffer)
add_subdirectory(libs/spsc_ring_queue)
add_subdirectory(libs/mpsc_ring_queue)
add_subdirectory(libs/weighted_set)
add_subdirectory(libs/endian)
add_subdirectory(libs/transport)

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if (OBZ_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
