# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_weighted_set INTERFACE)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::weighted_set ALIAS obz_weighted_set)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_weighted_set
    INTERFACE
        cxx_std_20
)

obz_install_library(obz_weighted_set weighted_set "${CMAKE_CURRENT_SOURCE_DIR}/include")
