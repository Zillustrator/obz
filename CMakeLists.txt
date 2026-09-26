cmake_minimum_required(VERSION 3.21)

# ------------------------------------------------------------------------------
# Project
# ------------------------------------------------------------------------------

project(obz VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(OBZ_INSTALL_CMAKEDIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/obz"
    CACHE STRING
    "Directory used to install the obz CMake package"
)

function(obz_install_library target export_name include_directory)
    get_target_property(target_type ${target} TYPE)

    if (target_type STREQUAL "INTERFACE_LIBRARY")
        set(include_scope INTERFACE)
    else()
        set(include_scope PUBLIC)
    endif()

    target_include_directories(${target}
        ${include_scope}
            $<BUILD_INTERFACE:${include_directory}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    set_target_properties(${target}
        PROPERTIES
            EXPORT_NAME ${export_name}
    )

    install(
        TARGETS ${target}
        EXPORT obzTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(
        DIRECTORY "${include_directory}/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endfunction()

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(OBZ_BUILD_TESTS "Build obz tests" ON)
option(OBZ_BUILD_EXAMPLES "Build obz examples" ON)
option(OBZ_ENABLE_ASAN_UBSAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if (OBZ_ENABLE_ASAN_UBSAN)
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
        message(FATAL_ERROR "OBZ_ENABLE_ASAN_UBSAN requires GCC or Clang")
    endif()

    function(obz_enable_asan_ubsan target)
        target_compile_options(${target}
            PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
                -fno-sanitize-recover=undefined
        )

        target_link_options(${target}
            PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
                -fno-sanitize-recover=undefined
        )
    endfunction()
endif()

# ------------------------------------------------------------------------------
# Libraries
# ------------------------------------------------------------------------------

add_subdirectory(libs/detail)
add_subdirectory(libs/blocking_queue)
add_subdirectory(libs/bounded_blocking_queue)
add_subdirectory(libs/ring_buffer)
add_subdirectory(libs/spsc_ring_queue)
add_subdirectory(libs/mpsc_ring_queue)
add_subdirectory(libs/weighted_set)
add_subdirectory(libs/endian)
add_subdirectory(libs/emotional_state)
add_subdirectory(libs/transport)
add_subdirectory(libs/thread_affinity)

# ------------------------------------------------------------------------------
# Package
# ------------------------------------------------------------------------------

configure_package_config_file(
    cmake/obzConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/obzConfig.cmake"
    INSTALL_DESTINATION ${OBZ_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/obzConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    EXPORT obzTargets
    FILE obzTargets.cmake
    NAMESPACE obz::
    DESTINATION ${OBZ_INSTALL_CMAKEDIR}
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/obzConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/obzConfigVersion.cmake"
    DESTINATION ${OBZ_INSTALL_CMAKEDIR}
)

# ------------------------------------------------------------------------------
# Tests
# ------------------------------------------------------------------------------

if (OBZ_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
