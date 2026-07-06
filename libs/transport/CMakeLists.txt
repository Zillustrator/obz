# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

add_library(obz_transport
    src/tcp_listener.cpp
    src/tcp_socket.cpp
    src/udp_socket.cpp
)

# ------------------------------------------------------------------------------
# Public alias
# ------------------------------------------------------------------------------

add_library(obz::transport ALIAS obz_transport)

# ------------------------------------------------------------------------------
# Compile features
# ------------------------------------------------------------------------------

target_compile_features(obz_transport
    PUBLIC
        cxx_std_20
)

if (WIN32)
    target_sources(obz_transport
        PRIVATE
            src/platform/win32_socket_platform.cpp
    )

    target_link_libraries(obz_transport
        PRIVATE
            ws2_32
    )
else()
    target_sources(obz_transport
        PRIVATE
            src/platform/posix_socket_platform.cpp
    )
endif()

obz_install_library(obz_transport transport "${CMAKE_CURRENT_SOURCE_DIR}/include")
