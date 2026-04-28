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
)

# ------------------------------------------------------------------------------
# Link test dependencies
# ------------------------------------------------------------------------------

target_link_libraries(obz_tests
    PRIVATE
        obz::blocking_queue
        obz::bounded_blocking_queue
        Catch2::Catch2WithMain
)

# ------------------------------------------------------------------------------
# CTest integration
# ------------------------------------------------------------------------------

include(Catch)
catch_discover_tests(obz_tests)