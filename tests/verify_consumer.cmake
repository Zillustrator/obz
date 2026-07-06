file(REMOVE_RECURSE
    "${OBZ_CONSUMER_BINARY_DIR}"
    "${OBZ_SOURCE_CONSUMER_BINARY_DIR}"
    "${OBZ_INSTALL_PREFIX}"
)

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        --install "${OBZ_BINARY_DIR}"
        --prefix "${OBZ_INSTALL_PREFIX}"
        --config "${OBZ_CONFIG}"
    RESULT_VARIABLE install_result
)

if (NOT install_result EQUAL 0)
    message(FATAL_ERROR "Failed to install obz for the consumer test")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        -S "${OBZ_CONSUMER_SOURCE_DIR}"
        -B "${OBZ_CONSUMER_BINARY_DIR}"
        -G "${OBZ_GENERATOR}"
        "-DCMAKE_PREFIX_PATH=${OBZ_INSTALL_PREFIX}"
    RESULT_VARIABLE configure_result
)

if (NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Failed to configure the obz consumer test")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        --build "${OBZ_CONSUMER_BINARY_DIR}"
        --config "${OBZ_CONFIG}"
    RESULT_VARIABLE build_result
)

if (NOT build_result EQUAL 0)
    message(FATAL_ERROR "Failed to build the obz consumer test")
endif()

execute_process(
    COMMAND
        "${CMAKE_CTEST_COMMAND}"
        --test-dir "${OBZ_CONSUMER_BINARY_DIR}"
        --build-config "${OBZ_CONFIG}"
        --output-on-failure
    RESULT_VARIABLE test_result
)

if (NOT test_result EQUAL 0)
    message(FATAL_ERROR "The installed-package obz consumer test failed")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        -S "${OBZ_CONSUMER_SOURCE_DIR}"
        -B "${OBZ_SOURCE_CONSUMER_BINARY_DIR}"
        -G "${OBZ_GENERATOR}"
        -DOBZ_CONSUMER_USE_FETCHCONTENT=ON
        "-DOBZ_SOURCE_DIR=${OBZ_SOURCE_DIR}"
    RESULT_VARIABLE source_configure_result
)

if (NOT source_configure_result EQUAL 0)
    message(FATAL_ERROR "Failed to configure the source-dependency obz consumer test")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        --build "${OBZ_SOURCE_CONSUMER_BINARY_DIR}"
        --config "${OBZ_CONFIG}"
    RESULT_VARIABLE source_build_result
)

if (NOT source_build_result EQUAL 0)
    message(FATAL_ERROR "Failed to build the source-dependency obz consumer test")
endif()

execute_process(
    COMMAND
        "${CMAKE_CTEST_COMMAND}"
        --test-dir "${OBZ_SOURCE_CONSUMER_BINARY_DIR}"
        --build-config "${OBZ_CONFIG}"
        --output-on-failure
    RESULT_VARIABLE source_test_result
)

if (NOT source_test_result EQUAL 0)
    message(FATAL_ERROR "The source-dependency obz consumer test failed")
endif()
