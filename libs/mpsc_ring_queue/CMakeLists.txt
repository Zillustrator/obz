# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_mpsc_ring_queue INTERFACE)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::mpsc_ring_queue ALIAS obz_mpsc_ring_queue)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_mpsc_ring_queue
    INTERFACE
        cxx_std_20
)

obz_install_library(obz_mpsc_ring_queue mpsc_ring_queue "${CMAKE_CURRENT_SOURCE_DIR}/include")
