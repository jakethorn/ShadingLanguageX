//
// Created by jaket on 04/06/2026.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <filesystem>
#include <string>

#include "compile.h"
#include "CompileOptions.h"

namespace py = pybind11;

PYBIND11_MODULE(_mxslc, m)
{
    m.doc() = "Python bindings for mxslc";

    py::class_<mxslc::CompileOptions>(m, "CompileOptions")
        .def(py::init<>())
        .def_readwrite("version", &mxslc::CompileOptions::version)
        .def_readwrite("reduce_graph", &mxslc::CompileOptions::reduce_graph)
        .def_property(
            "output_file",
            [](const mxslc::CompileOptions& opts) -> py::object
            {
                if (!opts.output_file)
                {
                    return py::none();
                }

                return py::cast(*opts.output_file);
            },
            [](mxslc::CompileOptions& opts, py::object value)
            {
                if (value.is_none())
                {
                    opts.output_file = std::nullopt;
                }
                else
                {
                    opts.output_file = value.cast<std::filesystem::path>();
                }
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
        py::arg("options")
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
        py::arg("options")
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
        py::arg("options")
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
        py::arg("options")
    );

    m.def(
        "compile_string_to_file",
        py::overload_cast<const std::string&, const std::filesystem::path&>(&mxslc::compile_to_file),
        py::arg("source"),
        py::arg("dst_path")
    );
}
