# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_bounded_blocking_queue INTERFACE)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::bounded_blocking_queue ALIAS obz_bounded_blocking_queue)

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------

target_include_directories(obz_bounded_blocking_queue
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_bounded_blocking_queue
    INTERFACE
        cxx_std_20
)