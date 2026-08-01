//
// Created by jaket on 24/06/2026.
//

#include "Decompiler_bindings.h"

#include "decompile/Decompiler.h"

void bind_decompiler(py::module_& m)
{
    py::class_<Decompiler>(m, "Decompiler")
        .def(py::init<const fs::path&>(), py::arg("src_path"))
        .def(py::init<const string&>(), py::arg("source"))
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
