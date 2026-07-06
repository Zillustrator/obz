cmake_minimum_required(VERSION 3.21)

project(obz_consumer LANGUAGES CXX)

option(OBZ_CONSUMER_USE_FETCHCONTENT "Consume ObzLib from source using FetchContent" OFF)

if (OBZ_CONSUMER_USE_FETCHCONTENT)
    if (NOT OBZ_SOURCE_DIR)
        message(FATAL_ERROR "OBZ_SOURCE_DIR is required when using FetchContent")
    endif()

    include(FetchContent)

    set(OBZ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(OBZ_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        obz
        SOURCE_DIR "${OBZ_SOURCE_DIR}"
    )

    FetchContent_MakeAvailable(obz)
else()
    find_package(obz 0.1 CONFIG REQUIRED)
endif()

add_executable(obz_consumer
    main.cpp
)

target_link_libraries(obz_consumer
    PRIVATE
        obz::blocking_queue
        obz::endian
        obz::thread_affinity
        obz::transport
)

enable_testing()
add_test(NAME obz_consumer_runs COMMAND obz_consumer)
