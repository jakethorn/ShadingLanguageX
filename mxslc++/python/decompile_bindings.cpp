//
// Created by jaket on 24/06/2026.
//

#include "decompile_bindings.h"

#include "decompile/decompile.h"
#include "decompile/DecompileOptions.h"

void bind_decompile_options(py::module_& m)
{
    py::class_<DecompileOptions>(m, "DecompileOptions")
        .def(py::init<>())
        .def_readwrite("emit_function_modifiers", &DecompileOptions::emit_function_modifiers);
}

void bind_decompile_functions(py::module_& m)
{
    m.def(
        "decompile_file_to_string",
        py::overload_cast<const fs::path&, const DecompileOptions&>(&decompile_to_string),
        py::arg("src_path"),
        py::arg("options") = DecompileOptions{}
    );

    m.def(
        "decompile_string_to_string",
        py::overload_cast<const string&, const DecompileOptions&>(&decompile_to_string),
        py::arg("source"),
        py::arg("options") = DecompileOptions{}
    );

    m.def(
        "decompile_file_to_file",
        py::overload_cast<const fs::path&, const std::optional<fs::path>&, const DecompileOptions&>(&decompile_to_file),
        py::arg("src_path"),
        py::arg("dst_path") = std::nullopt,
        py::arg("options") = DecompileOptions{}
    );

    m.def(
        "decompile_file_to_file",
        py::overload_cast<const fs::path&, const fs::path&, const DecompileOptions&>(&decompile_to_file),
        py::arg("src_path"),
        py::arg("dst_path"),
        py::arg("options") = DecompileOptions{}
    );

    m.def(
        "decompile_string_to_file",
        py::overload_cast<const string&, const fs::path&, const DecompileOptions&>(&decompile_to_file),
        py::arg("source"),
        py::arg("dst_path"),
        py::arg("options") = DecompileOptions{}
    );
}
