# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_bounded_blocking_queue INTERFACE)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::bounded_blocking_queue ALIAS obz_bounded_blocking_queue)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_bounded_blocking_queue
    INTERFACE
        cxx_std_20
)

obz_install_library(
    obz_bounded_blocking_queue
    bounded_blocking_queue
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
