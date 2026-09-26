# Shared implementation headers used transitively by public library targets.
add_library(obz_detail INTERFACE)
add_library(obz::detail ALIAS obz_detail)

target_compile_features(obz_detail INTERFACE cxx_std_20)

obz_install_library(obz_detail detail "${CMAKE_CURRENT_SOURCE_DIR}/include")
