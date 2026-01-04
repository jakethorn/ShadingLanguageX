cmake_minimum_required(VERSION 4.1)
project(mxslc)

if(MINGW)
    add_link_options("-Wl,--allow-multiple-definition")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------- MaterialX --------
set(MTLX_ROOT "C:/Users/jaket/Documents/Repos/Local/MaterialX-1.39.4/cmake-build-debug/installed")
set(MTLX_INCLUDE_DIR "${MTLX_ROOT}/include")
set(MTLX_LIB_DIR "${MTLX_ROOT}/lib")
find_library(MTLX_CORE_LIB MaterialXCore PATHS "${MTLX_LIB_DIR}" REQUIRED)
find_library(MTLX_FORMAT_LIB MaterialXFormat PATHS "${MTLX_LIB_DIR}" REQUIRED)

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

# -------- main --------
add_executable(mxslc main.cpp)

target_include_directories(mxslc PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(mxslc PRIVATE
        mtlx_lib
        mxslc_lib
)

# -------- tests --------
enable_testing()
add_subdirectory(tests)

# -------- static linkage --------
if(MINGW)
    target_compile_options(mxslc PRIVATE -static-libgcc -static-libstdc++)
    target_link_options(mxslc PRIVATE -static -pthread)
endif()
