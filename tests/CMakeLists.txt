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
    ../libs/endian/tests/endian_tests.cpp
    ../libs/emotional_state/tests/emotional_state_tests.cpp
    ../libs/transport/tests/transport_tests.cpp
    ../libs/thread_affinity/tests/thread_affinity_tests.cpp
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
        obz::endian
        obz::emotional_state
        obz::transport
        obz::thread_affinity
        Catch2::Catch2WithMain
)

# ------------------------------------------------------------------------------
# CTest integration
# ------------------------------------------------------------------------------

include(Catch)
catch_discover_tests(obz_tests)

add_test(
    NAME obz_package_consumer
    COMMAND ${CMAKE_COMMAND}
        -DOBZ_BINARY_DIR=${CMAKE_BINARY_DIR}
        -DOBZ_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DOBZ_CONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/consumer
        -DOBZ_CONSUMER_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/consumer-build
        -DOBZ_SOURCE_CONSUMER_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/source-consumer-build
        -DOBZ_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/consumer-install
        -DOBZ_GENERATOR=${CMAKE_GENERATOR}
        -DOBZ_CONFIG=$<CONFIG>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/verify_consumer.cmake
)
