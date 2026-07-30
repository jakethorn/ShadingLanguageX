//
// Created by jaket on 24/06/2026.
//

#include "decompile_bindings.h"

#include "decompile/decompile.h"

void bind_decompile_functions(py::module_& m)
{
    m.def(
        "decompile_file_to_string",
        py::overload_cast<const fs::path&>(&decompile_to_string),
        py::arg("src_path")
    );

    m.def(
        "decompile_string_to_string",
        py::overload_cast<const string&>(&decompile_to_string),
        py::arg("source")
    );

    m.def(
        "decompile_file_to_file",
        py::overload_cast<const fs::path&, const std::optional<fs::path>&>(&decompile_to_file),
        py::arg("src_path"),
        py::arg("dst_path") = std::nullopt
    );

    m.def(
        "decompile_file_to_file",
        py::overload_cast<const fs::path&, const fs::path&>(&decompile_to_file),
        py::arg("src_path"),
        py::arg("dst_path")
    );

    m.def(
        "decompile_string_to_file",
        py::overload_cast<const string&, const fs::path&>(&decompile_to_file),
        py::arg("source"),
        py::arg("dst_path")
    );
}
