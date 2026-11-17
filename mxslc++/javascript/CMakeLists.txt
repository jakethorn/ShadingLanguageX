# -------- mxslc JavaScript (Emscripten) bindings --------
# This directory is only added when the project is configured with
# -DBUILD_JS=ON and a valid Emscripten SDK path. See the root CMakeLists.txt.

# Linker flags for the Emscripten module.
set(JS_LINK_FLAGS " -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s EXPORT_NAME='Mxslc' \
    -s MEMORY_GROWTH_LINEAR_STEP=32MB \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s FORCE_FILESYSTEM=1 \
    --preload-file ${PROJECT_SOURCE_DIR}/libraries@libraries \
    --bind ")

# TypeScript declarations (bin/JsMxslc.d.ts) generated from the embind
# bindings. Emscripten needs tsc on the PATH for this; build_javascript.sh
# provides it from this folder's node_modules (npm install).
find_program(TSC_EXECUTABLE tsc)
if (TSC_EXECUTABLE)
    string(APPEND JS_LINK_FLAGS " --emit-tsd JsMxslc.d.ts ")
else()
    message(WARNING "JS: tsc not found on the PATH, JsMxslc.d.ts will not be generated. Run 'npm install' in javascript/ and add javascript/node_modules/.bin to the PATH.")
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "JS: Building Debug")
    string(APPEND JS_LINK_FLAGS " -s ASSERTIONS=1 ")
else()
    message(STATUS "JS: Building Release")
    string(APPEND JS_LINK_FLAGS " -Os ")
endif()

add_executable(JsMxslc JsMxslc.cpp)

set_target_properties(JsMxslc
    PROPERTIES
    OUTPUT_NAME JsMxslc
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
    LINK_FLAGS "${JS_LINK_FLAGS}"
)

target_include_directories(JsMxslc PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(JsMxslc PRIVATE
    mtlx_lib
    mxslc_lib
)
