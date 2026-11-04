//
// Created by jaket on 24/06/2026.
//

#include "compile_bindings.h"

#include "compile.h"
#include "CompileOptions.h"

void bind_compile_functions(py::module_& m)
{
    m.def(
        "compile_file_to_string",
        py::overload_cast<const fs::path&>(&compile_to_string),
        py::arg("src_path")
    );

    m.def(
        "compile_file_to_string",
        py::overload_cast<const fs::path&, const CompileOptions&>(&compile_to_string),
        py::arg("src_path"),
        py::arg("opts")
    );

    m.def(
        "compile_string_to_string",
        py::overload_cast<const string&>(&compile_to_string),
        py::arg("source")
    );

    m.def(
        "compile_string_to_string",
        py::overload_cast<const string&, const CompileOptions&>(&compile_to_string),
        py::arg("source"),
        py::arg("opts")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const fs::path&>(&compile_to_file),
        py::arg("src_path")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const fs::path&, const CompileOptions&>(&compile_to_file),
        py::arg("src_path"),
        py::arg("opts")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const fs::path&, const fs::path&>(&compile_to_file),
        py::arg("src_path"),
        py::arg("dst_path")
    );

    m.def(
        "compile_string_to_file",
        py::overload_cast<const string&, const CompileOptions&>(&compile_to_file),
        py::arg("source"),
        py::arg("opts")
    );

    m.def(
        "compile_string_to_file",
        py::overload_cast<const string&, const fs::path&>(&compile_to_file),
        py::arg("source"),
        py::arg("dst_path")
    );
}
