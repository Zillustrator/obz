# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_blocking_queue INTERFACE)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::blocking_queue ALIAS obz_blocking_queue)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_blocking_queue
    INTERFACE
        cxx_std_20
)

obz_install_library(obz_blocking_queue blocking_queue "${CMAKE_CURRENT_SOURCE_DIR}/include")
