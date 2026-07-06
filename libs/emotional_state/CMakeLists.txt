# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_emotional_state INTERFACE)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::emotional_state ALIAS obz_emotional_state)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_emotional_state
    INTERFACE
        cxx_std_20
)

obz_install_library(obz_emotional_state emotional_state "${CMAKE_CURRENT_SOURCE_DIR}/include")
