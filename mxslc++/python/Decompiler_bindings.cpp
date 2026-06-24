//
// Created by jaket on 24/06/2026.
//

#include "Decompiler_bindings.h"

void bind_decompiler(py::module_& m)
{
    py::class_<mxslc::Decompiler>(m, "Decompiler")
        .def(py::init<const std::filesystem::path&>(), py::arg("src_path"))
        .def(py::init<const std::string&>(), py::arg("source"))
        .def(
            "decompile_document",
            &mxslc::Decompiler::decompile_document
        )
        .def(
            "decompile_node",
            py::overload_cast<const std::string&, bool>(&mxslc::Decompiler::decompile_node),
            py::arg("node_name"),
            py::arg("with_dependencies") = false
        )
        .def(
            "decompile_node_def",
            py::overload_cast<const std::string&, bool>(&mxslc::Decompiler::decompile_node_def),
            py::arg("node_def_name"),
            py::arg("with_dependencies") = false
        )
        .def(
            "decompile_node_graph",
            py::overload_cast<const std::string&, bool>(&mxslc::Decompiler::decompile_node_graph),
            py::arg("node_graph_name"),
            py::arg("with_dependencies") = false
        );
}
