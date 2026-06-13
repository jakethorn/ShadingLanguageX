find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_mxslc
        python_bindings.cpp
        utils.cpp
)

target_include_directories(_mxslc PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(_mxslc PRIVATE
        mtlx_lib
        mxslc_lib
)

install(TARGETS _mxslc
        LIBRARY DESTINATION mxslc
        RUNTIME DESTINATION mxslc
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/libraries"
        DESTINATION mxslc
)