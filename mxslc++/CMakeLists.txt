cmake_minimum_required(VERSION 3.28.3)

# -------- JavaScript (Emscripten) setup --------
# Must be configured before project() so the Emscripten toolchain file can be
# applied for the whole build. Building the JavaScript/WebAssembly bindings
# requires the Emscripten SDK and a wasm build of MaterialX (MTLX_ROOT).
option(BUILD_JS "Build the mxslc JavaScript (WebAssembly) bindings. Requires the emscripten environment." OFF)
set(MXLSC_EMSDK_PATH "" CACHE PATH "Path to EMSDK (e.g. '~/emsdk').")
if (BUILD_JS)
    if (EXISTS "${MXLSC_EMSDK_PATH}")
        set(EMSDK_PATH ${MXLSC_EMSDK_PATH})
    elseif (EXISTS $ENV{EMSDK})
        set(EMSDK_PATH $ENV{EMSDK})
    endif()
    if (EMSDK_PATH)
        set(CMAKE_TOOLCHAIN_FILE "${EMSDK_PATH}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
    else()
        message("The EMSDK path was not supplied, disabling BUILD_JS")
        set(BUILD_JS OFF)
    endif()
endif()

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

# The Emscripten toolchain restricts find_* searches to its own sysroot. Allow
# the wasm MaterialX install (MTLX_ROOT) to be located for the JS build.
if(BUILD_JS)
        list(APPEND CMAKE_FIND_ROOT_PATH "${MTLX_ROOT}")
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
endif()

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

# -------- javascript bindings --------
if(BUILD_JS)
        add_subdirectory(javascript)
endif()

# -------- native mxslc executable --------
if(NOT BUILD_JS)
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
endif()

# -------- tests --------
include(CTest)

if(BUILD_TESTING AND NOT BUILD_JS)
        add_subdirectory(tests)
endif()
