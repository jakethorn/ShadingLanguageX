# -------- fetch gtest --------
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)

# Prevent gtest from overriding parent project compiler settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# -------- mxslc_tests --------
add_executable(mxslc_tests
        span_tests.cpp
        parse_args_tests.cpp
        groundtruth_tests.cpp
        failure_tests.cpp
        error_message_tests.cpp
        reduce_graph_tests.cpp
        primitive_type_tests.cpp
        primitive_op_tests.cpp
        primitive_utils_tests.cpp
        primitive_to_string_tests.cpp
        global_tests.cpp
        overwrite_data_files_tests.cpp
        compile_options_sources_tests.cpp
        materialx_library_cache_tests.cpp
        value_override_tests.cpp
        compile_api_tests.cpp
        default_function_behaviour_tests.cpp
        decompile/roundtrip_tests.cpp
        decompile/groundtruth_roundtrip_diff.cpp
        decompile/groundtruth_decompile_tests.cpp
        decompile/decompile_dependencies_tests.cpp
        decompile/decompile_library_names_tests.cpp
        decompile/decompiler_api_tests.cpp
        decompile/decompile_cases_tests.cpp
        decompile/decompile_dropped_elements_tests.cpp
        decompile/decompile_port_names_tests.cpp
        decompile/decompile_readability_tests.cpp
)

target_link_libraries(mxslc_tests PRIVATE
        mxslc_lib
        GTest::gtest
        GTest::gtest_main
)

# the test utils, e.g., utils/comp_utils.h, are also included by the tests in subdirectories
target_include_directories(mxslc_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(mxslc_tests PRIVATE
        TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)

include(GoogleTest)
gtest_discover_tests(mxslc_tests)

# -------- Copy libraries directory --------
add_custom_command(
        TARGET mxslc_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/libraries"
        "$<TARGET_FILE_DIR:mxslc_tests>/libraries"
        COMMENT "Copying libraries directory for mxslc_tests"
)