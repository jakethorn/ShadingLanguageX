//
// Created by jaket on 04/06/2026.
//

#include <pybind11/pybind11.h>

#include "common.h"
#include "utils.h"

PYBIND11_MODULE(_mxslc, m)
{
    m.doc() = "Python bindings for mxslc";

    py::class_<mxslc::CompileOptions>(m, "CompileOptions")
            .def(py::init([](
                std::optional<std::filesystem::path> output_file,
                std::string version,
                std::optional<std::string> func_name,
                const py::list& func_args,
                const bool reduce_graph)
            {
                auto opts = std::make_unique<mxslc::CompileOptions>();
                opts->output_file = std::move(output_file);
                opts->version = std::move(version);
                opts->func_name = std::move(func_name);
                opts->reduce_graph = reduce_graph;

                for (py::handle py_obj : func_args)
                    opts->func_args.push_back(utils::marshall_primitive(py_obj));

                return opts;
            }),
            py::arg("output_file") = std::nullopt,
            py::arg("version") = mxslc::CompileOptions{}.version,
            py::arg("func_name") = std::nullopt,
            py::arg("func_args") = py::list(),
            py::arg("reduce_graph") = mxslc::CompileOptions{}.reduce_graph
        )
        .def_readwrite("output_file", &mxslc::CompileOptions::output_file)
        .def_readwrite("version", &mxslc::CompileOptions::version)
        .def_readwrite("reduce_graph", &mxslc::CompileOptions::reduce_graph)
        .def_readwrite("func_name", &mxslc::CompileOptions::func_name)
        .def_property(
            "func_args",
            // getter
            nullptr,
            // setter
            [](mxslc::CompileOptions& opts, const py::list& py_args) {
                opts.func_args.clear();
                for (py::handle py_obj : py_args)
                    opts.func_args.push_back(utils::marshall_primitive(py_obj));
            }
        );

    m.def(
        "compile_file_to_string",
        py::overload_cast<const std::filesystem::path&>(&mxslc::compile_to_string),
        py::arg("src_path")
    );

    m.def(
        "compile_file_to_string",
        py::overload_cast<const std::filesystem::path&, const mxslc::CompileOptions&>(&mxslc::compile_to_string),
        py::arg("src_path"),
        py::arg("opts")
    );

    m.def(
        "compile_string_to_string",
        py::overload_cast<const std::string&>(&mxslc::compile_to_string),
        py::arg("source")
    );

    m.def(
        "compile_string_to_string",
        py::overload_cast<const std::string&, const mxslc::CompileOptions&>(&mxslc::compile_to_string),
        py::arg("source"),
        py::arg("opts")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const std::filesystem::path&>(&mxslc::compile_to_file),
        py::arg("src_path")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const std::filesystem::path&, const mxslc::CompileOptions&>(&mxslc::compile_to_file),
        py::arg("src_path"),
        py::arg("opts")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const std::filesystem::path&, const std::filesystem::path&>(&mxslc::compile_to_file),
        py::arg("src_path"),
        py::arg("dst_path")
    );

    m.def(
        "compile_string_to_file",
        py::overload_cast<const std::string&, const mxslc::CompileOptions&>(&mxslc::compile_to_file),
        py::arg("source"),
        py::arg("opts")
    );

    m.def(
        "compile_string_to_file",
        py::overload_cast<const std::string&, const std::filesystem::path&>(&mxslc::compile_to_file),
        py::arg("source"),
        py::arg("dst_path")
    );
}
