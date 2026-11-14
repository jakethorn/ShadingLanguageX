//
// Created by jaket on 24/06/2026.
//

#include "Decompiler_bindings.h"

#include "decompile/Decompiler.h"

void bind_decompiler(py::module_& m)
{
    // pybind11 converts every str to a path, so the string constructor must come first; it reads a str that is not
    // MaterialX source as a path, e.g., Decompiler("example.mtlx"), and pathlib.Path objects go to the path constructor
    py::class_<Decompiler>(m, "Decompiler")
        .def(py::init<const string&>(), py::arg("source"))
        .def(py::init<const fs::path&>(), py::arg("src_path"))
        .def(
            "decompile_document",
            &Decompiler::decompile_document
        )
        .def(
            "decompile_node",
            py::overload_cast<const string&, bool>(&Decompiler::decompile_node),
            py::arg("node_name"),
            py::arg("with_dependencies") = false
        )
        .def(
            "decompile_node_def",
            py::overload_cast<const string&, bool>(&Decompiler::decompile_node_def),
            py::arg("node_def_name"),
            py::arg("with_dependencies") = false
        )
        .def(
            "decompile_node_graph",
            py::overload_cast<const string&, bool>(&Decompiler::decompile_node_graph),
            py::arg("node_graph_name"),
            py::arg("with_dependencies") = false
        );
}
