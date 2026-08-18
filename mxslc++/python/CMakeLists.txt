find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_mxslc
        python_bindings.cpp
        CompileOptions_bindings.cpp
        Variable_bindings.cpp
        compile_bindings.cpp
        decompile_bindings.cpp
        Decompiler_bindings.cpp
        Macro_bindings.cpp
        utils.cpp
)

target_include_directories(_mxslc PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(_mxslc PRIVATE
        mtlx_lib
        mxslc_lib
)

if(DEFINED SKBUILD_PROJECT_VERSION)
        target_compile_definitions(_mxslc PRIVATE MXSLCXX_PY_VERSION="${SKBUILD_PROJECT_VERSION}")
endif()

install(TARGETS _mxslc
        LIBRARY DESTINATION mxslc
        RUNTIME DESTINATION mxslc
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/libraries"
        DESTINATION mxslc
)
