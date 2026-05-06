# ------------------------------------------------------------------------------
# Test dependencies
# ------------------------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)

FetchContent_MakeAvailable(Catch2)

# ------------------------------------------------------------------------------
# Test executable
# ------------------------------------------------------------------------------

add_executable(obz_tests
    ../libs/blocking_queue/tests/blocking_queue_tests.cpp
    ../libs/bounded_blocking_queue/tests/bounded_blocking_queue_tests.cpp
    ../libs/ring_buffer/tests/ring_buffer_tests.cpp
    ../libs/spsc_ring_queue/tests/spsc_ring_queue_tests.cpp
    ../libs/mpsc_ring_queue/tests/mpsc_ring_queue_tests.cpp
    ../libs/weighted_set/tests/weighted_set_tests.cpp
)

# ------------------------------------------------------------------------------
# Link test dependencies
# ------------------------------------------------------------------------------

target_link_libraries(obz_tests
    PRIVATE
        obz::blocking_queue
        obz::bounded_blocking_queue
        obz::ring_buffer
        obz::spsc_ring_queue
        obz::mpsc_ring_queue
        obz::weighted_set
        Catch2::Catch2WithMain
)

# ------------------------------------------------------------------------------
# CTest integration
# ------------------------------------------------------------------------------

include(Catch)
catch_discover_tests(obz_tests)
