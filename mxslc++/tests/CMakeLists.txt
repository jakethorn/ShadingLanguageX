# include gtest headers
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(mxslc_tests
        span_tests.cpp
        parse_args_tests.cpp
        groundtruth_tests.cpp
        type_tests.cpp
)

target_link_libraries(mxslc_tests PRIVATE
        mxslc_lib
        gtest
        gtest_main
)

target_include_directories(mxslc_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/source
)

target_compile_definitions(mxslc_tests PRIVATE
        TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)
