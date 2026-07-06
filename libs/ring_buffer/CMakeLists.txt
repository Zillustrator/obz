# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_ring_buffer INTERFACE)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::ring_buffer ALIAS obz_ring_buffer)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_ring_buffer
    INTERFACE
        cxx_std_20
)

obz_install_library(obz_ring_buffer ring_buffer "${CMAKE_CURRENT_SOURCE_DIR}/include")
