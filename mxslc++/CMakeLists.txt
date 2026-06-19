cmake_minimum_required(VERSION 3.28.3)
project(mxslc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------- MaterialX --------
set(MTLX_ROOT "" CACHE PATH "Path to installed MaterialX directory")
message(STATUS "MTLX_ROOT = ${MTLX_ROOT}")

if(NOT MTLX_ROOT)
        message(FATAL_ERROR
                "MTLX_ROOT is not set. Please configure with "
                "-DMTLX_ROOT=/path/to/MaterialX/install"
        )
endif()

set(MTLX_INCLUDE_DIR "${MTLX_ROOT}/include")
set(MTLX_LIB_DIR "${MTLX_ROOT}/lib")
message(STATUS "MTLX_INCLUDE_DIR = ${MTLX_INCLUDE_DIR}")
message(STATUS "MTLX_LIB_DIR = ${MTLX_LIB_DIR}")

find_library(MTLX_CORE_LIB MaterialXCore PATHS "${MTLX_LIB_DIR}" REQUIRED)
find_library(MTLX_FORMAT_LIB MaterialXFormat PATHS "${MTLX_LIB_DIR}" REQUIRED)
message(STATUS "MTLX_CORE_LIB = ${MTLX_CORE_LIB}")
message(STATUS "MTLX_FORMAT_LIB = ${MTLX_FORMAT_LIB}")

add_library(mtlx_lib INTERFACE)

target_include_directories(mtlx_lib INTERFACE
        "${MTLX_INCLUDE_DIR}"
)

target_link_libraries(mtlx_lib INTERFACE
        "${MTLX_FORMAT_LIB}"
        "${MTLX_CORE_LIB}"
)

# -------- source --------
add_subdirectory(source)

# -------- python bindings --------
if(DEFINED SKBUILD)
        add_subdirectory(python)
endif()

# -------- main --------
add_executable(mxslc main.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(mxslc PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
        target_link_options(mxslc PRIVATE -fsanitize=address,undefined)
endif()

target_include_directories(mxslc PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(mxslc PRIVATE
        mtlx_lib
        mxslc_lib
)

# -------- Copy libraries directory --------
add_custom_command(
        TARGET mxslc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/libraries"
        "$<TARGET_FILE_DIR:mxslc>/libraries"
        COMMENT "Copying libraries directory"
)

# -------- tests --------
include(CTest)

if(BUILD_TESTING)
        add_subdirectory(tests)
endif()
