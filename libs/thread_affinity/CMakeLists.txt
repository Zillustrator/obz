# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_thread_affinity
    src/thread_affinity.cpp
)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::thread_affinity ALIAS obz_thread_affinity)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_thread_affinity
    PUBLIC
        cxx_std_20
)

if (WIN32)
    target_sources(obz_thread_affinity
        PRIVATE
            src/platform/win32_thread_affinity.cpp
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Threads REQUIRED)

    target_sources(obz_thread_affinity
        PRIVATE
            src/platform/linux_thread_affinity.cpp
    )

    target_compile_definitions(obz_thread_affinity
        PRIVATE
            _GNU_SOURCE
    )

    target_link_libraries(obz_thread_affinity
        PRIVATE
            Threads::Threads
    )
else()
    target_sources(obz_thread_affinity
        PRIVATE
            src/platform/unsupported_thread_affinity.cpp
    )
endif()

obz_install_library(obz_thread_affinity thread_affinity "${CMAKE_CURRENT_SOURCE_DIR}/include")
