# include gtest headers
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(mxslc_tests
        #utils_tests.cpp
        #scan_tests.cpp
        #tokenreader_tests.cpp
        span_tests.cpp
        #parse_attribute_tests.cpp
        #parse_expression_tests.cpp
        #param_tests.cpp
        #scope_func_tests.cpp
        #token_tests.cpp
        #materialx_library_tests.cpp
        parse_args_tests.cpp
)

target_link_libraries(mxslc_tests PRIVATE
        mxslc_lib
        gtest
        gtest_main
)

target_include_directories(mxslc_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../source
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/runtime
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/expressions
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/statements
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/utils
)

target_compile_definitions(mxslc_tests PRIVATE
        TEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)
